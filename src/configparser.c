#include "configparser.h"
#include "filesystems.h"
#include <efi.h>
#include <efilib.h>
#include <string.h>

#define MAX_LINE_SIZE 256
#define PROTOCOLS_COUNT 4

#define CONFIG_PARAM_LINUX (CHAR8 *)"linux"
#define CONFIG_PARAM_INITRD (CHAR8 *)"initrd"
#define CONFIG_PARAM_CMDLINE (CHAR8 *)"cmdline"
#define CONFIG_PARAM_PROTOCOL (CHAR8 *)"protocol"

CHAR8 *BootProtocolTypes[PROTOCOLS_COUNT] = {(CHAR8 *)"", (CHAR8 *)"efi_chainload", (CHAR8 *)"efi_handover",
                                             (CHAR8 *)"linux64"};

BOOT_PROTOCOL GetBootProtocol(CHAR8 *ProtocolName)
{
    for (INT8 Index = 0; Index < PROTOCOLS_COUNT; Index++)
    {
        if (!strcmpa(ProtocolName, BootProtocolTypes[Index]))
        {
            return (BOOT_PROTOCOL)Index;
        }
    }
    return BOOT_PROTOCOL_NOT_SPECIFIED;
}

VOID ConvertToUnicode(CHAR16 *UnicodeString, CHAR8 *AsciiString)
{
    CHAR8 *Src = AsciiString;
    CHAR16 *Dest = UnicodeString;
    while (*Src)
    {
        *Dest++ = *Src++;
    }
    *Dest = L'\0';
}

VOID AsciiStrCpy(CHAR8 *DestString, CHAR8 *SrcString)
{
    CHAR8 *Src = SrcString;
    CHAR8 *Dest = DestString;
    while (*Src)
    {
        *Dest++ = *Src++;
    }
    *Dest = '\0';
}
CHAR8 *ReadConfig(EFI_FILE_HANDLE Volume, CHAR16 *FileName)
{
    EFI_FILE_HANDLE FileHandle;

    EFI_STATUS Status;
    Status = Volume->Open(Volume, &FileHandle, FileName, EFI_FILE_MODE_READ,
                          EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

    if (EFI_ERROR(Status))
    {
        Print(L"File open error\n");
        return NULL;
    }

    INT64 Size = FileSize(FileHandle);

    if (Size < 0)
    {
        return NULL;
    }

    UINT64 ReadSize = Size;
    CHAR8 *ConfigContents = AllocateZeroPool(Size + 2);

    FileHandle->Read(FileHandle, &ReadSize, ConfigContents);
    if (ConfigContents[Size - 1] != '\n')
    {
        ConfigContents[Size - 1] = '\n';
    }
    FileHandle->Close(FileHandle);

    return ConfigContents;
}

VOID ProcessParamValue(BOOTLOADER_CONFIG *Config, CHAR8 *Param, CHAR8 *Value)
{
    if (!strcmpa(Param, CONFIG_PARAM_LINUX))
    {
        ConvertToUnicode(Config->Linux, Value);
    }
    else if (!strcmpa(Param, CONFIG_PARAM_INITRD))
    {
        ConvertToUnicode(Config->InitRd, Value);
    }
    else if (!strcmpa(Param, CONFIG_PARAM_CMDLINE))
    {
        AsciiStrCpy(Config->CommandLine, Value);
    }
    else if (!strcmpa(Param, CONFIG_PARAM_PROTOCOL))
    {
        Config->Protocol = GetBootProtocol(Value);
    }
}

CHAR8 *SkipWhiteSpace(CHAR8 *Str)
{
    while (*Str && (*Str == '\t' || *Str == ' '))
    {
        *Str = '\0';
        Str++;
    }
    return Str;
}

CHAR8 *SkipWhiteSpaceBack(CHAR8 *Str, CHAR8 *Limit)
{
    while (Str > Limit && (*Str == '\t' || *Str == '\n'))
    {
        *Str = '\0';
        Str--;
    }
    return Str;
}

CHAR8 *GetLine(CHAR8 *Data, CHAR8 **Next)
{
    CHAR8 *Start = Data;
    if (!*Data)
    {
        return 0;
    }
    while (*Data != '\n')
    {
        Data++;
        if (!*Data)
        {
            *Next = NULL;
            goto RemoveR;
        }
    }
    *Next = Data + 1;
RemoveR:
    if (*Data == '\n')
    {
        *Data = '\0';
        if ((Data - Start) >= 2 && Data[-1] == '\r')
        {
            Data[-1] = '\0';
        }
    }

    return Start;
}

VOID ProcessLine(BOOTLOADER_CONFIG *Config, CHAR8 *Line)
{
    CHAR8 *Initial = Line;
    Line = SkipWhiteSpace(Line);
    CHAR8 *Param = Line;
    CHAR8 Current = *Line;
    while (Current && Current > 'a' && Current < 'z')
    {
        Line++;
        Current = *Line;
    }
    if (!*Line)
        return;
    *Line = '\0';
    Line = SkipWhiteSpace(++Line);
    CHAR8 *Value = Line;
    while (Current)
    {
        Line++;
        Current = *Line;
    }

    SkipWhiteSpaceBack(Line, Initial);

    ProcessParamValue(Config, Param, Value);
}

INT16 Parse(CHAR8 *ConfigContents, BOOTLOADER_CONFIG *Config)
{
    CHAR8 *Next;
    CHAR8 *Line = GetLine(ConfigContents, &Next);
    while (Line)
    {
        if (*Line == '#')
        {
            Line = GetLine(Next, &Next);
            continue;
        }
        else
        {
            ProcessLine(Config, Line);
            Line = GetLine(Next, &Next);
        }
    }
    return 0;
}

BOOTLOADER_CONFIG *ParseConfig(EFI_FILE_HANDLE Volume, CHAR16 *FileName)
{
    Print(L"Parsing config \'%s\' ...\n", FileName);
    CHAR8 *ConfigContents = ReadConfig(Volume, FileName);
    BOOTLOADER_CONFIG *BootloaderConfig = AllocateZeroPool(sizeof(BOOTLOADER_CONFIG));
    Parse(ConfigContents, BootloaderConfig);
    return BootloaderConfig;
}
