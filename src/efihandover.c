#include "bootloader.h"
#include "common.h"
#include "linuxtypes.h"
#include <efi.h>
#include <efilib.h>

typedef void (*HANDOVER_FUNCTION)(EFI_HANDLE, EFI_SYSTEM_TABLE *, BOOT_PARAMS *);

EFI_STATUS EfiHandOverJump(EFI_HANDLE Image, EFI_PHYSICAL_ADDRESS KernelAddress, BOOT_PARAMS *BootParams)
{
    Print(L"Starting Linux...");
    HANDOVER_FUNCTION HandoverFunction;

    asm volatile("cli");

    UINT32 HandoverOffset = BootParams->hdr.handover_offset;

    HandoverFunction = (HANDOVER_FUNCTION)(KernelAddress + 512 + HandoverOffset);
    HandoverFunction(Image, gST, BootParams);
    return EFI_NOT_STARTED;
}

BOOTLOADER_STATUS EfiHandOverBoot(BOOT_CONTEXT *BootContext)
{
    VOID *Kernel = NULL;
    VOID *InitRd = NULL;
    VOID *CommandLine = NULL;
    EFI_STATUS Status;

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
    Status = EfiHandOverJump(BootContext->Image, (EFI_PHYSICAL_ADDRESS)Kernel, &BootContext->BootParams);
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
