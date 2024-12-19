#include "memory.h"
#include <efi.h>
#include <efilib.h>
#include <string.h>

EFI_STATUS GetMemoryMap(MEMORY_MAP *MemoryMap)
{
    // Get initial memory map size (send 0 for map size)
    EFI_STATUS status = EFI_SUCCESS;
    MemoryMap->Size = 0;
    status = gBS->GetMemoryMap(&MemoryMap->Size, NULL, &MemoryMap->Key, &MemoryMap->DescriptorSize,
                               &MemoryMap->DescriptorVersion);

    if (EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL)
    {
        Print(L"Could not get initial memory map size.\n");
        return status;
    }

    // Allocate buffer for actual memory map for size in mmap->size;
    //   need to allocate enough space for an additional memory descriptor or 8 in the map due to
    //   this allocation itself.
    MemoryMap->Size += MemoryMap->DescriptorSize * 8;
    MemoryMap->Map = AllocateZeroPool(MemoryMap->Size);

    // Call get memory map again to get the actual memory map now that the buffer is the correct
    //   size
    status = gBS->GetMemoryMap(&MemoryMap->Size, MemoryMap->Map, &MemoryMap->Key, &MemoryMap->DescriptorSize,
                               &MemoryMap->DescriptorVersion);

    if (EFI_ERROR(status))
    {
        Print(L"Could not get UEFI memory map! %d\n", status);
        return status;
    }
    return status;
}

EFI_STATUS AllocateMemoryAligned(UINTN Size, UINTN Align, EFI_PHYSICAL_ADDRESS *Address)
{
    EFI_STATUS Status;
    UINTN NumberOfPages = EFI_SIZE_TO_PAGES(Size);
    MEMORY_MAP MemoryMap;

    Status = GetMemoryMap(&MemoryMap);
    if (Status != EFI_SUCCESS)
        return Status;

    UINTN Descriptor = (UINTN)MemoryMap.Map;
    UINTN EndOfMap = (UINTN)MemoryMap.Map + MemoryMap.Size;

    for (; Descriptor < EndOfMap; Descriptor += MemoryMap.DescriptorSize)
    {
        EFI_MEMORY_DESCRIPTOR *CurrentDescriptor = (EFI_MEMORY_DESCRIPTOR *)Descriptor;
        if (CurrentDescriptor->Type != EfiConventionalMemory)
            continue;

        if (CurrentDescriptor->NumberOfPages < NumberOfPages)
            continue;

        EFI_PHYSICAL_ADDRESS MemoryRegionStart = CurrentDescriptor->PhysicalStart;
        EFI_PHYSICAL_ADDRESS MemoryRegionEnd = MemoryRegionStart + (CurrentDescriptor->NumberOfPages << EFI_PAGE_SHIFT);

        /* Do not allocate in low memory */
        if (MemoryRegionEnd <= 1 << 20)
            continue;

        EFI_PHYSICAL_ADDRESS AlignedAddress = (MemoryRegionStart + Align - 1) & ~(Align - 1);

        if ((AlignedAddress + Size) <= MemoryRegionEnd)
        {
            Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, NumberOfPages, &AlignedAddress);
            if (Status == EFI_SUCCESS)
            {
                *Address = AlignedAddress;
                break;
            }
        }
    }

    if (Descriptor == EndOfMap)
        Status = EFI_OUT_OF_RESOURCES;

    gBS->FreePool(MemoryMap.Map);

    return Status;
}

void FreeMemory(EFI_PHYSICAL_ADDRESS Address, UINTN Size)
{
    UINTN NumberOfPages = EFI_SIZE_TO_PAGES(Size);
    gBS->FreePages(Address, NumberOfPages);
}
