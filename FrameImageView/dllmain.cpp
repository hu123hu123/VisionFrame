// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    (void)hModule;
    (void)ul_reason_for_call;
    (void)lpReserved;
    return TRUE;
}