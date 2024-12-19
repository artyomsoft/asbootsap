#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "linuxtypes.h"

typedef enum
{
    BOOT_PROTOCOL_NOT_SPECIFIED = 0,
    BOOT_PROTOCOL_EFI_CHAINLOAD = 1,
    BOOT_PROTOCOL_EFI_HANDOVER = 2,
    BOOT_PROTOCOL_LINUX64 = 3
} BOOT_PROTOCOL;

typedef enum
{
    BOOTLOADER_SUCCESS,
    BOOTLOADER_ERROR
} BOOTLOADER_STATUS;

typedef struct
{
    CHAR16 Linux[256];
    CHAR16 InitRd[256];
    CHAR8 CommandLine[4096];
    BOOT_PROTOCOL Protocol;
} BOOTLOADER_CONFIG;

typedef struct
{
    EFI_HANDLE Image;
    EFI_FILE_HANDLE Root;
    BOOTLOADER_CONFIG *Config;
    BOOT_PARAMS BootParams __attribute__((aligned(4096)));
} BOOT_CONTEXT;

extern BOOTLOADER_STATUS Start(BOOT_CONTEXT *BootContext);

#endif