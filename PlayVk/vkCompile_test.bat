@echo off

:: prompt to set environment variable
if "%VK_SDK_PATH%"=="" (
    echo Please set the VK_SDK_PATH environment variable to point to your Vulkan SDK installation.
    pause
    exit /b
)

set GLSLC=%VK_SDK_PATH%\Bin\glslc.exe
set SHADER_DIR=shaders

:: if success
if not exist "%GLSLC%" (
    echo glslc.exe not found at %GLSLC%. Please check your Vulkan SDK path or ensure the compiler is available.
    pause
    exit /b
)

:: compile 
"%GLSLC%" %SHADER_DIR%\vert.glsl.vert -o %SHADER_DIR%\bin\vert.spv 
"%GLSLC%" %SHADER_DIR%\frag.glsl.frag -o %SHADER_DIR%\bin\frag.spv

echo Shader compilation complete.
::pause
