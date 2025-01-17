// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Windowsx.h>
//#include <Shlobj.h>

// Standard definitions:
#include <ole2.h>
#include <wrl/client.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ File I/O
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstdio>

// C++ Auxiliary
#include <ctime>
#include <vector>
#include <iterator>
#include <algorithm>

// DirectX Math
#include <DirectXMath.h>

// Kinect Header Files:
#include <NuiApi.h>

// Direct2D Header Files
#include <d2d1.h>

// Direct3D 11 Header Files
#include <d3d11_1.h>
#include <directxcolors.h>

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}