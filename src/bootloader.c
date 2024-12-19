#include "bootloader.h"
#include "chainload.h"
#include "efihandover.h"
#include "linuxboot64.h"
#include <efilib.h>

BOOTLOADER_STATUS Start(BOOT_CONTEXT *BootContext)
{
    switch (BootContext->Config->Protocol)
    {
    case BOOT_PROTOCOL_EFI_CHAINLOAD:
        return ChainLoadBoot(BootContext);
    case BOOT_PROTOCOL_LINUX64: {
        return Linux64Boot(BootContext);
    }
    case BOOT_PROTOCOL_EFI_HANDOVER: {
        return EfiHandOverBoot(BootContext);
    }
    default:
        Print(L"Unsupported Linux Booting Protocol");
    }
    return BOOTLOADER_ERROR;
}
