#include "bootloader.h"
#include "configparser.h"
#include "filesystems.h"
#include <efi.h>
#include <efilib.h>
#ifdef EFI_DEBUG
#include "debugutils.h"
#endif

#define BOOTLOADER_CONFIG_FILE_NAME L"\\asbootsap.cfg"

BOOT_CONTEXT *InitEfiBootContext(EFI_HANDLE Image)
{
    EFI_FILE_HANDLE Root = GetRootDir(Image);

    if (!Root)
    {
        Print(L"Could not get root dir \n");
    }
    BOOT_CONTEXT *BootContext = AllocateZeroPool(sizeof(BOOT_CONTEXT));
    BOOTLOADER_CONFIG *Config = ParseConfig(Root, BOOTLOADER_CONFIG_FILE_NAME);
    BootContext->Image = Image;
    BootContext->Root = Root;
    BootContext->Config = Config;
    return BootContext;
}

BOOT_PROTOCOL SelectBootProtocol()
{
    Print(L"1 - Chain Load\n");
    Print(L"2 - EFI HANDOVER\n");
    Print(L"3 - Linux Boot Protocol\n");

    EFI_INPUT_KEY Key;
    UINTN KeyEvent = 0;
    while ((UINTN)Key.ScanCode != SCAN_ESC)
    {
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &KeyEvent);
        gST->ConOut->ClearScreen(gST->ConOut);
        gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
        gST->ConIn->Reset(gST->ConIn, FALSE);
        switch (Key.UnicodeChar)
        {
        case L'1':
            Print(L"Chain Loading...\n");
            return BOOT_PROTOCOL_EFI_CHAINLOAD;
        case L'2':
            Print(L"Efi Handover...\n");
            return BOOT_PROTOCOL_EFI_HANDOVER;
        case L'3':
            Print(L"Linux Boot Protocol...\n");
            return BOOT_PROTOCOL_LINUX64;
        }
    }
    gST->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return BOOT_PROTOCOL_NOT_SPECIFIED;
}

EFI_STATUS
efi_main(EFI_HANDLE Image, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(Image, SystemTable);

#ifdef EFI_DEBUG
    OutputLoadedAddress(Image);
    PlaceBreakPoint();
#endif
    gST->ConOut->ClearScreen(gST->ConOut);
    Print(L"Firmware Vendor: %s\n", SystemTable->FirmwareVendor);
    Print(L"Firmware Revision %x\n", SystemTable->FirmwareRevision);
    Print(L"UEFI Version: %d.%d \n", (SystemTable->Hdr.Revision & 0xffff0000) >> 16,
          SystemTable->Hdr.Revision & 0x0000ffff);

    BOOT_CONTEXT *BootContext = InitEfiBootContext(Image);
    if (BootContext->Config->Protocol == BOOT_PROTOCOL_NOT_SPECIFIED)
    {
        BootContext->Config->Protocol = SelectBootProtocol();
    }
    if (Start(BootContext) != BOOTLOADER_SUCCESS)
    {
        Print(L"Can not boot OS\n");
        for (;;)
            ;
    }
    return EFI_SUCCESS;
}
