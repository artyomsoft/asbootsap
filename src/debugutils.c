#include "debugutils.h"

extern char _text, _data;

VOID OutputLoadedAddress(EFI_HANDLE Image)
{
    Print(L"-exec add-symbol-file build/asbootsap.debug 0x%08x -s .data 0x%08x\n", &_text, &_data);
}

VOID PlaceBreakPoint()
{
    int wait = 1;
    while (wait)
    {
        __asm__ __volatile__("pause");
    }
}
