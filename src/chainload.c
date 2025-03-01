#include "bootloader.h"
#include <efi.h>
#include <efilib.h>

static CHAR16 *ConvertCommandLineToLoadOptions(CHAR16 *InitRdFileName, CHAR8 *CommandLine)
{
    CHAR8 *InitRd = (CHAR8 *)"initrd=";
    UINTN len = (strlena(InitRd) + strlena(CommandLine) + 1 + StrLen(InitRdFileName) + 1) * 2;
    CHAR16 *Result = AllocateZeroPool(len);
    CHAR16 *Current = Result;
    CHAR8 *Char;
    CHAR16 *Char16;
    Char = InitRd;

    for (; *Char; Char++, Current++)
    {
        *Current = *Char;
    }

    Print(L"%s\n", InitRdFileName);

    Char16 = InitRdFileName;
    for (; *Char16; Char16++, Current++)
    {
        *Current = *Char16;
    }

    *Current++ = L' ';

    Char = CommandLine;

    for (; *Char; Char++, Current++)
    {
        *Current = *Char;
    }

    //    *Current++ = 0;

    Print(L"%s", Result);
    return Result;
}

EFI_DEVICE_PATH *GetDevicePath(EFI_HANDLE Image, CHAR16 *FileName)
{
    Print(L"Getting Device Path...\n");

    EFI_STATUS status = 0;

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

    // Get the loaded image protocol for the current image
    gBS->OpenProtocol(Image, &LoadedImageProtocol, (VOID **)&LoadedImage, Image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status))
    {
        Print(L"Failed to get LoadedImageProtocol!\nReason: %r\n", status);
        return NULL;
    }

    EFI_DEVICE_PATH *DevicePath = NULL;

    // Create a device path for the file
    DevicePath = FileDevicePath(LoadedImage->DeviceHandle, FileName);
    if (DevicePath == NULL)
    {
        Print(L"Failed to create device path for file!\n");
        return NULL;
    }

    Print(L"DevicePath: %s\n", DevicePathToStr(DevicePathFromHandle(LoadedImage->DeviceHandle)));
    return DevicePath;
}

EFI_STATUS LoadLinux(EFI_HANDLE BootLoaderHandle, EFI_DEVICE_PATH *DevicePath, EFI_HANDLE *LinuxImageHandle)
{
    Print(L"Loading Linux Image...\n");

    EFI_STATUS Status = EFI_SUCCESS;

    Status = gBS->LoadImage(FALSE, BootLoaderHandle, DevicePath, NULL, 0, LinuxImageHandle);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to Load Linux Image\n");
        return Status;
    }
    return Status;
}

EFI_STATUS StartLinuxEfiImage(EFI_HANDLE *LinuxImageHandle)
{
    Print(L"Starting Linux Efi Image ...\n");

    EFI_STATUS status = 0;

    status = gBS->StartImage(LinuxImageHandle, (UINTN *)NULL, (CHAR16 **)NULL);
    if (EFI_ERROR(status))
    {
        Print(L"Stating image error: %r\n", status);
        return status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS SetupLoadOptions(EFI_HANDLE LinuxImageHandle, CHAR16 *InitRdFileName, CHAR8 *CommandLine)
{
    Print(L"Setting up loading options...\n");

    EFI_LOADED_IMAGE_PROTOCOL *LinuxImage;

    EFI_STATUS status = gBS->OpenProtocol(LinuxImageHandle, &LoadedImageProtocol, (VOID **)&LinuxImage,
                                          LinuxImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status))
    {
        Print(L"Failed to get LoadedImageProtocol!\nReason: %r\n", status);
        return status;
    }

    CHAR16 *LoadOptions = ConvertCommandLineToLoadOptions(InitRdFileName, CommandLine);

    LinuxImage->LoadOptions = LoadOptions;
    LinuxImage->LoadOptionsSize = StrSize(LoadOptions);

    return EFI_SUCCESS;
}

BOOTLOADER_STATUS ChainLoadBoot(BOOT_CONTEXT *BootContext)
{
    EFI_DEVICE_PATH *DevicePath;
    EFI_HANDLE LinuxImageHandle;

    if (!(DevicePath = GetDevicePath(BootContext->Image, BootContext->Config->Linux)))
    {
        return BOOTLOADER_ERROR;
    }

    if (EFI_ERROR(LoadLinux(BootContext->Image, DevicePath, &LinuxImageHandle)))
    {
        return BOOTLOADER_ERROR;
    }

    if (EFI_ERROR(SetupLoadOptions(LinuxImageHandle, BootContext->Config->InitRd, BootContext->Config->CommandLine)))
    {
        return BOOTLOADER_ERROR;
    }

    if (EFI_ERROR(StartLinuxEfiImage(LinuxImageHandle)))
    {
        return BOOTLOADER_ERROR;
    }
    // Never reach the code below
    return BOOTLOADER_SUCCESS;
}