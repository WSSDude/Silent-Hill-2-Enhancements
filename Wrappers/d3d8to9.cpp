/**
* Copyright (C) 2019 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "External\d3d8to9\source\d3d8to9.hpp"
#include "External\d3d8to9\source\d3dx9.hpp"
#include "External\Hooking\Hook.h"
#include "Common\Utils.h"
#include "wrapper.h"
#include "d3d8to9.h"
#include "Logging\Logging.h"
#include "BuildNo.rc"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define APP_VERSION TOSTRING(FILEVERSION)

typedef LPDIRECT3D9(WINAPI *PFN_Direct3DCreate9)(UINT SDKVersion);

FARPROC p_Direct3DCreate9 = nullptr;

PFN_D3DXAssembleShader D3DXAssembleShader = nullptr;
PFN_D3DXDisassembleShader D3DXDisassembleShader = nullptr;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = nullptr;

// Redirects or hooks 'Direct3DCreate8' to go to d3d8to9
void EnableD3d8to9()
{
	HMODULE d3d9_dll = LoadLibrary(L"d3d9.dll");
	if (!d3d9_dll)
	{
		Logging::Log() << __FUNCTION__ << " Error: could not load d3d9.dll!";
		return;
	}

	AddHandleToVector(d3d9_dll);
	p_Direct3DCreate9 = GetProcAddress(d3d9_dll, "Direct3DCreate9");
	d3d8::Direct3DCreate8_var = p_Direct3DCreate8to9;
}

// Handles calls to 'Direct3DCreate8' for d3d8to9
Direct3D8 *WINAPI Direct3DCreate8to9(UINT SDKVersion)
{
	UNREFERENCED_PARAMETER(SDKVersion);

	LOG_ONCE("Starting D3d8to9 v" << APP_VERSION);

	Logging::Log() << "Redirecting 'Direct3DCreate8' to --> 'Direct3DCreate9' (" << SDKVersion << ")";

	// Declare Direct3DCreate9
	static PFN_Direct3DCreate9 Direct3DCreate9 = reinterpret_cast<PFN_Direct3DCreate9>(p_Direct3DCreate9);
	if (!Direct3DCreate9)
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!";
		return nullptr;
	}

	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d == nullptr)
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create 'Direct3DCreate9'!";
		return nullptr;
	}

	// Load D3DX
	if (!D3DXAssembleShader || !D3DXDisassembleShader || !D3DXLoadSurfaceFromSurface)
	{
		// Declare module vars
		static HMODULE d3dx9Module = nullptr;

		// Declare d3dx9_xx.dll name
		wchar_t d3dx9name[MAX_PATH] = { 0 };

		// Declare d3dx9_xx.dll version
		for (int x = 99; x > 9 && !d3dx9Module; x--)
		{
			// Get dll name
			swprintf_s(d3dx9name, L"d3dx9_%d.dll", x);
			// Load dll
			d3dx9Module = LoadLibrary(d3dx9name);
		}

		if (d3dx9Module)
		{
			D3DXAssembleShader = reinterpret_cast<PFN_D3DXAssembleShader>(GetProcAddress(d3dx9Module, "D3DXAssembleShader"));
			D3DXDisassembleShader = reinterpret_cast<PFN_D3DXDisassembleShader>(GetProcAddress(d3dx9Module, "D3DXDisassembleShader"));
			D3DXLoadSurfaceFromSurface = reinterpret_cast<PFN_D3DXLoadSurfaceFromSurface>(GetProcAddress(d3dx9Module, "D3DXLoadSurfaceFromSurface"));
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to load d3dx9_xx.dll! Some features will not work correctly.";
		}
	}

	// Check if WineD3D is loaded
	if (GetModuleHandle(L"libwine.dll") || GetModuleHandle(L"wined3d.dll"))
	{
		Logging::Log() << __FUNCTION__ << " Warning: WineD3D detected!  It is not recommended to use WineD3D with Silent Hill 2 Enhancements.";
	}

	return new Direct3D8(d3d);
}
