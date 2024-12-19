#include "bootloader.h"

extern VOID *LoadKernel(BOOT_PARAMS *BootParams, EFI_FILE_HANDLE Volume, CHAR16 *FileName);
extern VOID UnloadKernel(VOID *Kernel);

extern VOID *LoadInitRd(BOOT_PARAMS *BootParams, EFI_FILE_HANDLE Volume, CHAR16 *FileName);
extern VOID UnloadInitRd(VOID *InitRd);

extern VOID *SetupCommandLine(BOOT_CONTEXT *BootContext, CHAR8 *CommandLine);
extern VOID FreeCommandLine(VOID *CommandLine);
