// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <atlbase.h>

// C++ 
#include <vector>
#include <string>

// add lua script support
#ifdef __cplusplus 
extern "C" {
#endif
	// put all lua headfiles here
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

#ifdef _DEBUG
#define LUATRACE(s)    printf(s)
#define POPMSGBOX(format, ...)  { char buff[MAX_PATH]={0};   \
	int position = 0;             \
	_snprintf(buff, MAX_PATH-1, "File: %s\r\nLine: %d\r\n\r\n", __FILE__, __LINE__);   \
	position = strlen(buff);   \
	_snprintf(buff + position, MAX_PATH-1-position, format, __VA_ARGS__); \
	MessageBoxA(NULL, buff, "a error occured...", MB_OK|MB_ICONEXCLAMATION);   \
}
#else
#define LUATRACE(s)    
#define POPMSGBOX(format, ...)   
#endif

#pragma warning( disable : 4996 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4267 )
#pragma warning( disable : 4290 )

#include <vector>
#include <string>
#include <utility>
#include <algorithm>

// TODO: reference additional headers your program requires here
