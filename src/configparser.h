#ifndef __CONFIGPARSER_H__
#define __CONFIGPARSER_H__

#include "bootloader.h"
#include <efi.h>

extern BOOTLOADER_CONFIG *ParseConfig(EFI_FILE_HANDLE Volume, CHAR16 *FileName);

#endif
