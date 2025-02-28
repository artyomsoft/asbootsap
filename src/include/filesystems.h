#include <efi.h>

extern EFI_FILE_HANDLE GetRootDir(EFI_HANDLE image);
extern INT64 FileSize(EFI_FILE_HANDLE FileHandle);