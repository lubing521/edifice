// cinderella.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "SerialPort4Lua.h"
#include "EIBCommNEO.h"
#include "TheDeviceObj.h"

#ifdef _MANAGED 
#pragma managed(push, off)
#endif

std::auto_ptr<TheDeviceObj> g_devobj;

static int g_dll_attached_count;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if ( DLL_PROCESS_ATTACH == ul_reason_for_call )
	{
		if ( 0 == g_dll_attached_count )
		{
		}
		
		g_dll_attached_count++;
	}
	else if ( DLL_PROCESS_DETACH == ul_reason_for_call )
	{
		g_dll_attached_count--;
		if ( 0 == g_dll_attached_count )
		{
		}
	}

    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// 从lua环境载入dll. 
int luaopen_cinderella(lua_State* L)
{
	if ( g_dll_attached_count > 1 )
	{
		return 0;
	}

	register_funs(L);

	return 0;
}
