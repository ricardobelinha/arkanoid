// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DLL.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		initializeResourcesDLL();
		break;
    case DLL_PROCESS_DETACH:
		closeResourcesDLL();
        break;
    }
    return TRUE;
}

