#include "mcbeinject.h"

void WinMainCRTStartup(void)
{
    // If one were to be pedanctic, one might pass in the module handle, the command line and whether to display a 
    // window. However, there is no point in doing so when we will not need those values for the forseeable future.
    int res = wWinMain(NULL, NULL, NULL, 0);
    ExitProcess(res);

}

void* memset(void* dst, int c, unsigned int len)
{
    unsigned char* p = (unsigned char*)dst;

    while (len--)
    {
        *p++ = (unsigned char)c;
    }

    return dst;
}