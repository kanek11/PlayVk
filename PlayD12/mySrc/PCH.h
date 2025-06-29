#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h> 
#include <dxcapi.h>  // DXC headers
#include <d3d12shader.h> // for ID3D12ShaderReflection


#include <wrl.h>
#include <shellapi.h>

#include <vector>	
#include <string>
#include <iostream>

#include <unordered_map>
#include <optional>