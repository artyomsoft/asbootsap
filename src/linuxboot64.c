#include "bootloader.h"
#include "common.h"
#include "linuxtypes.h"
#include "memory.h"
#include <efi.h>
#include <efilib.h>
#include <string.h>

#define EFI_LOADER_SIGNATURE "EL64"

static EFI_STATUS SetupE820MemoryMap(BOOT_PARAMS *BootParams, MEMORY_MAP *MemoryMap);

typedef void (*KERNEL_FUNCTION)(VOID *, BOOT_PARAMS *);

extern void ReloadSegments();

typedef struct
{
    UINT16 Limit;
    UINT64 *Base;
} __attribute__((packed)) DESCRIPTOR;

UINT64 Gdt[] = {
    0, 0,
    0x00209b0000000000, // CODE
    0x0000930000000000, // DATA
};

static DESCRIPTOR GdtDescriptor __attribute__((aligned(4096))) = {Limit : sizeof(Gdt) - 1, Base : Gdt};

static DESCRIPTOR IdtDescriptor __attribute__((aligned(4096))) = {Limit : 0, Base : 0};

static VOID JumpToKernel(EFI_PHYSICAL_ADDRESS KernelStart, BOOT_PARAMS *BootParams)
{
    KERNEL_FUNCTION kf;

    /* The 64-bit kernel entry is 512 bytes after the start. */
    kf = (KERNEL_FUNCTION)KernelStart + 512;
    asm volatile("cli");
    asm volatile("lidt %0" ::"m"(IdtDescriptor));
    asm volatile("lgdt %0" ::"m"(GdtDescriptor));
    ReloadSegments();
    /*
     * The first parameter is a dummy because the kernel expects
     * boot_params in %[re]si.
     */
    kf(NULL, BootParams);
}

static void find_bits(unsigned long mask, UINT8 *pos, UINT8 *size)
{
    UINT8 first, len;

    first = 0;
    len = 0;

    if (mask)
    {
        while (!(mask & 0x1))
        {
            mask = mask >> 1;
            first++;
        }

        while (mask & 0x1)
        {
            mask = mask >> 1;
            len++;
        }
    }
    *pos = first;
    *size = len;
}

static EFI_STATUS SetupScreenInfo(SCREEN_INFO *si)
{
    Print(L"Setting up screen info...\n");
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    EFI_GUID graphics_proto = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_HANDLE *gop_handle = NULL;
    EFI_STATUS status;
    UINTN nr_gops;
    UINTN size;
    int i;

    status = gBS->LocateHandleBuffer(ByProtocol, &graphics_proto, NULL, &nr_gops, &gop_handle);
    if (EFI_ERROR(status))
    {
        return status;
    }
    for (i = 0; i < nr_gops; i++)
    {
        EFI_HANDLE *h = &gop_handle[i];

        status = gBS->HandleProtocol(*h, &graphics_proto, (void **)&gop);

        if (EFI_ERROR(status))
        {
            continue;
        }

        status = gop->QueryMode(gop, gop->Mode->Mode, &size, &info);
        if (status == EFI_SUCCESS)
        {
            break;
        }
    }

    /* We found a GOP */
    if (i != nr_gops)
    {
        /* EFI framebuffer */
        si->orig_video_isVGA = 0x70;

        si->orig_x = 0;
        si->orig_y = 0;
        si->orig_video_page = 0;
        si->orig_video_mode = 0;
        si->orig_video_cols = 0;
        si->orig_video_lines = 0;
        si->orig_video_ega_bx = 0;
        si->orig_video_points = 0;

        si->lfb_base = gop->Mode->FrameBufferBase;
        si->lfb_size = gop->Mode->FrameBufferSize;
        si->lfb_width = info->HorizontalResolution;
        si->lfb_height = info->VerticalResolution;
        si->pages = 1;
        si->vesapm_seg = 0;
        si->vesapm_off = 0;

        if (info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor)
        {
            si->lfb_depth = 32;
            si->red_size = 8;
            si->red_pos = 0;
            si->green_size = 8;
            si->green_pos = 8;
            si->blue_size = 8;
            si->blue_pos = 16;
            si->rsvd_size = 8;
            si->rsvd_pos = 24;
            si->lfb_linelength = info->PixelsPerScanLine * 4;
        }
        else if (info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor)
        {
            si->lfb_depth = 32;
            si->red_size = 8;
            si->red_pos = 16;
            si->green_size = 8;
            si->green_pos = 8;
            si->blue_size = 8;
            si->blue_pos = 0;
            si->rsvd_size = 8;
            si->rsvd_pos = 24;
            si->lfb_linelength = info->PixelsPerScanLine * 4;
        }
        else if (info->PixelFormat == PixelBitMask)
        {
            find_bits(info->PixelInformation.RedMask, &si->red_pos, &si->red_size);
            find_bits(info->PixelInformation.GreenMask, &si->green_pos, &si->green_size);
            find_bits(info->PixelInformation.BlueMask, &si->blue_pos, &si->blue_size);
            find_bits(info->PixelInformation.ReservedMask, &si->rsvd_pos, &si->rsvd_size);
            si->lfb_depth = si->red_size + si->green_size + si->blue_size + si->rsvd_size;
            si->lfb_linelength = (info->PixelsPerScanLine * si->lfb_depth) / 8;
        }
        else
        {
            si->lfb_depth = 4;
            si->red_size = 0;
            si->red_pos = 0;
            si->green_size = 0;
            si->green_pos = 0;
            si->blue_size = 0;
            si->blue_pos = 0;
            si->rsvd_size = 0;
            si->rsvd_pos = 0;
            si->lfb_linelength = si->lfb_width / 2;
        }
    }
    return status;
}

static EFI_STATUS SetupEfiInfo(BOOT_PARAMS *BootParams, MEMORY_MAP *EfiMemoryMap)
{
    Print(L"Setting up EFI info...\n");
    EFI_INFO *EfiInfo = &BootParams->efi_info;
    memcpy(&EfiInfo->efi_loager_signature, EFI_LOADER_SIGNATURE, sizeof(EfiInfo->efi_loager_signature));
    UINT64 SystemTable = (UINT64)gST;
    EFI_MEMORY_DESCRIPTOR *mm = EfiMemoryMap->Map;
    EfiInfo->efi_memdesc_size = EfiMemoryMap->DescriptorSize;
    EfiInfo->efi_memdesc_version = EfiMemoryMap->DescriptorVersion;
    EfiInfo->efi_memmap_size = EfiMemoryMap->Size;
    EfiInfo->efi_memmap = (UINT32)((UINT64)mm);
    EfiInfo->efi_memmap_hi = (UINT32)((UINT64)mm >> 32);
    EfiInfo->efi_systab = (UINT32)SystemTable;
    EfiInfo->efi_systab_hi = (UINT32)(SystemTable >> 32);
    return EFI_SUCCESS;
}

static EFI_STATUS TryExitBootServices(EFI_HANDLE Bootloader, MEMORY_MAP *MemoryMap)
{
    EFI_STATUS Status;
    Print(L"Exiting Boot Services...\n");
    Status = gBS->ExitBootServices(Bootloader, MemoryMap->Key);

    UINTN Retries = 0;
    const UINTN MAX_RETRIES = 5;
    while (EFI_ERROR(Status) && Retries < MAX_RETRIES)
    {
        // firmware could do a partial shutdown, need to get memory map again
        //   and try exit boot services again
        FreePool(MemoryMap->Map);
        Status = GetMemoryMap(MemoryMap);
        if (EFI_ERROR(Status))
            return Status;
        Retries++;
    }

    if (Retries == MAX_RETRIES)
    {
        Print(L"Could not Exit Boot Services!\n");
        return EFI_INVALID_PARAMETER;
    }

    return Status;
}
static EFI_STATUS TransferControlToLinux(EFI_HANDLE Bootloader, VOID *Kernel, BOOT_PARAMS *BootParams)
{
    EFI_STATUS Status = EFI_SUCCESS;
    MEMORY_MAP MemoryMap;

    Print(L"Terminating boot services...\n");
    SetupScreenInfo(&BootParams->screen_info);
    GetMemoryMap(&MemoryMap);
    SetupE820MemoryMap(BootParams, &MemoryMap);
    SetupEfiInfo(BootParams, &MemoryMap);
    TryExitBootServices(Bootloader, &MemoryMap);
    JumpToKernel((EFI_PHYSICAL_ADDRESS)Kernel, BootParams);

    return Status;
}

static EFI_STATUS SetupE820MemoryMap(BOOT_PARAMS *BootParams, MEMORY_MAP *MemoryMap)
{
    Print(L"Setting up E820 memory map...\n");
    E820_ENTRY *E820Table = BootParams->e820_table;
    UINT16 ItemsCount = MemoryMap->Size / MemoryMap->DescriptorSize;
    EFI_MEMORY_DESCRIPTOR *Current = NULL;
    E820_ENTRY *PrevE820 = NULL;
    E820_ENTRY *CurrentE820 = NULL;

    unsigned int e820_type = 0;
    UINT32 cnt = 0;
    for (int i = 0; i < ItemsCount; i++)
    {
        Current = (((VOID *)MemoryMap->Map) + i * (MemoryMap->DescriptorSize));
        CurrentE820 = &E820Table[cnt];
        switch (Current->Type)
        {
        case EfiReservedMemoryType:
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
        case EfiPalCode:
            e820_type = E820_RESERVED;
            break;

        case EfiUnusableMemory:
            e820_type = E820_UNUSABLE;
            break;

        case EfiACPIReclaimMemory:
            e820_type = E820_ACPI;
            break;

        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory:
            e820_type = E820_RAM;
            break;

        case EfiACPIMemoryNVS:
            e820_type = E820_NVS;
            break;

        default:
            continue;
        }
        if (PrevE820 && PrevE820->Type == e820_type && (PrevE820->Address + PrevE820->Size) == Current->PhysicalStart)
        {
            PrevE820->Size += (Current->NumberOfPages << EFI_PAGE_SHIFT);
            continue;
        }

        CurrentE820->Address = Current->PhysicalStart;
        CurrentE820->Size = Current->NumberOfPages << EFI_PAGE_SHIFT;
        CurrentE820->Type = e820_type;
        PrevE820 = CurrentE820;
        cnt++;
        if (cnt > 128)
        {
            return EFI_BUFFER_TOO_SMALL;
        }
    }
    BootParams->e820_entries = cnt;
    return EFI_SUCCESS;
}

BOOTLOADER_STATUS Linux64Boot(BOOT_CONTEXT *BootContext)
{
    EFI_STATUS Status;
    VOID *Kernel = NULL;
    VOID *InitRd = NULL;
    VOID *CommandLine = NULL;

    Kernel = LoadKernel(&BootContext->BootParams, BootContext->Root, BootContext->Config->Linux);
    if (!Kernel)
    {
        goto Error;
    }
    InitRd = LoadInitRd(&BootContext->BootParams, BootContext->Root, BootContext->Config->InitRd);
    if (!InitRd)
    {
        goto Error;
    }
    CommandLine = SetupCommandLine(BootContext, BootContext->Config->CommandLine);
    if (!CommandLine)
    {
        goto Error;
    }
    Status = TransferControlToLinux(BootContext->Image, Kernel, &BootContext->BootParams);
    if (EFI_ERROR(Status))
    {
        goto Error;
    }
Error:
    FreeCommandLine(CommandLine);
    UnloadInitRd(InitRd);
    UnloadKernel(Kernel);
    return BOOTLOADER_ERROR;
}
