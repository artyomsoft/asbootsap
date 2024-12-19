#include <efi.h>
#include <efilib.h>

EFI_FILE_HANDLE GetRootDir(EFI_HANDLE image)
{
    EFI_LOADED_IMAGE *loaded_image = NULL;             /* image interface */
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID; /* image interface GUID */

    /* get the loaded image protocol interface for our "image" */
    BS->HandleProtocol(image, &lipGuid, (void **)&loaded_image);
    return LibOpenRoot(loaded_image->DeviceHandle);
}

INT64 FileSize(EFI_FILE_HANDLE FileHandle)
{
    UINT64 ret;
    EFI_FILE_INFO *FileInfo;
    FileInfo = LibFileInfo(FileHandle);
    ret = FileInfo->FileSize;
    FreePool(FileInfo);
    return ret;
}
