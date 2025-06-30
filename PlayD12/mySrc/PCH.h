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
#include <array>
#include <string>

#include <unordered_map>
#include <map> 

#include <iostream>
#include <sstream>
#include <fstream> 

#include <functional> //event system
#include <memory>     //smart pointers 
#include <algorithm>
#include <utility> 
#include <random>

// C++17 above 
#include <filesystem>
#include <source_location>

#include <variant>
#include <optional>
 
#include <format>
 