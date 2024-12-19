#include "bootloader.h"
#include "filesystems.h"
#include "memory.h"
#include <efilib.h>
#include <string.h>

static VOID *ReadKernelPayload(EFI_FILE_HANDLE FileHandle, SETUP_HEADER *Header)
{
    EFI_STATUS Status;

    UINT64 Offset = (Header->setup_sects + 1) * 512;

    Status = FileHandle->SetPosition(FileHandle, Offset);

    UINT64 BzImageSize = FileSize(FileHandle);

    UINT64 ReadSize = BzImageSize - Offset;

    EFI_PHYSICAL_ADDRESS Result = 0;

    Status = AllocateMemoryAligned(Header->init_size, Header->kernel_alignment, &Result);

    if (EFI_ERROR(Status))
    {
        Print(L"Error allocating memory for the kernel");
        return NULL;
    }

    Status = FileHandle->Read(FileHandle, &ReadSize, (VOID *)Result);

    if (EFI_ERROR(Status))
    {
        Print(L"Error loading kernel");
        FreeMemory(Result, Header->init_size);
        return NULL;
    }

    return (VOID *)Result;
}

static EFI_STATUS ReadKernelHeader(EFI_FILE_HANDLE FileHandle, SETUP_HEADER *Header)
{
    UINT64 ReadSize = 1024;
    INT8 BootSectors[1024];

    EFI_STATUS Status = FileHandle->Read(FileHandle, &ReadSize, BootSectors);

    if (EFI_ERROR(Status) || ReadSize != 1024)
    {
        Print(L"KernelRead error\n");
        return 1;
    }

    memcpy(Header, (SETUP_HEADER *)(BootSectors + 0x1f1), sizeof(SETUP_HEADER));

    return EFI_SUCCESS;
}

VOID *LoadKernel(BOOT_PARAMS *BootParams, EFI_FILE_HANDLE Volume, CHAR16 *FileName)
{
    EFI_FILE_HANDLE FileHandle;
    EFI_STATUS Status;
    Status = Volume->Open(Volume, &FileHandle, FileName, EFI_FILE_MODE_READ,
                          EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

    if (EFI_ERROR(Status))
    {
        Print(L"Can not open file\n");
        FileHandle->Close(FileHandle);
        return NULL;
    }

    Status = ReadKernelHeader(FileHandle, &BootParams->hdr);

    if (EFI_ERROR(Status))
    {
        Print(L"Error reading kernel header");
        FileHandle->Close(FileHandle);
        return NULL;
    }

    VOID *Kernel = ReadKernelPayload(FileHandle, &BootParams->hdr);

    BootParams->hdr.type_of_loader = 0xff;

    FileHandle->Close(FileHandle);

    return Kernel;
}

VOID *LoadInitRd(BOOT_PARAMS *BootParams, EFI_FILE_HANDLE Volume, CHAR16 *FileName)
{
    Print(L"Loading InitRd...\n");
    EFI_FILE_HANDLE FileHandle;
    EFI_STATUS status;
    status = Volume->Open(Volume, &FileHandle, FileName, EFI_FILE_MODE_READ,
                          EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

    if (EFI_ERROR(status))
    {
        Print(L"OpenVolume error\n");
        return NULL;
    }

    UINT64 ReadSize = FileSize(FileHandle);

    VOID *InitRdContents;

    status = AllocateMemoryAligned(ReadSize, 0x1000, (EFI_PHYSICAL_ADDRESS *)&InitRdContents);
    if (status != EFI_SUCCESS)
    {
        Print(L"Allocation memory for initrd error\n");
        return NULL;
    }

    InitRdContents = AllocateZeroPool(ReadSize);

    status = FileHandle->Read(FileHandle, &ReadSize, InitRdContents);
    if (EFI_ERROR(status))
    {
        Print(L"Read error\n");
        return NULL;
    }

    status = FileHandle->Close(FileHandle);

    if (EFI_ERROR(status))
    {
        Print(L"InitrdImage Read error\n");
        return NULL;
    }

    BootParams->hdr.ramdisk_image = (UINT32)(UINT64)InitRdContents;
    BootParams->ext_ramdisk_image = (UINT32)((UINT64)InitRdContents >> 32);
    BootParams->hdr.ramdisk_size = ReadSize;

    return InitRdContents;
}

VOID UnloadInitRd(VOID *InitRd)
{
    FreePool(InitRd);
}

VOID FreeCommandLine(VOID *CommandLine)
{
    if (CommandLine)
    {
        FreePool(CommandLine);
    }
}

VOID *SetupCommandLine(BOOT_CONTEXT *BootContext, CHAR8 *CommandLine)
{
    Print(L"Setting up the command line...\n");
    UINTN Size = strlena(CommandLine) + 1;
    UINT8 *Cmd = AllocateZeroPool(Size);
    memcpy(Cmd, CommandLine, Size);
    BootContext->BootParams.hdr.cmd_line_ptr = (UINT32)(UINT64)Cmd;
    BootContext->BootParams.ext_cmd_line_ptr = (UINT32)((UINT64)Cmd >> 32);
    return Cmd;
}
