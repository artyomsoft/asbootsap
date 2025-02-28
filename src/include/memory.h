#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "linuxtypes.h"
#include <efi.h>

#define E820_RAM 1
#define E820_RESERVED 2
#define E820_ACPI 3
#define E820_NVS 4
#define E820_UNUSABLE 5

extern EFI_STATUS GetMemoryMap(MEMORY_MAP *MemoryMap);

extern EFI_STATUS AllocateMemoryAligned(UINTN size, UINTN align, EFI_PHYSICAL_ADDRESS *addr);
extern void FreeMemory(EFI_PHYSICAL_ADDRESS Address, UINTN Size);

#endif
