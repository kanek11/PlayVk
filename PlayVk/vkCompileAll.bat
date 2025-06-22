@echo off

:: prompt to set environment variable
if "%VK_SDK_PATH%"=="" (
    echo Please set the VK_SDK_PATH environment variable to point to your Vulkan SDK installation.
    pause
    exit /b
)

set GLSLC=%VK_SDK_PATH%\Bin\glslc.exe
set SHADER_DIR=shaders
set INCLUDE_DIR=shaders\includes

:: if success
if not exist "%GLSLC%" (
    echo glslc.exe not found at %GLSLC%. Please check your Vulkan SDK path or ensure the compiler is available.
    pause
    exit /b
)

:: compile 
for /r %SHADER_DIR% %%f in (*.vert) do (
    "%GLSLC%" %%f -I %INCLUDE_DIR% -o %SHADER_DIR%\bin\%%~nf.spv || goto :CompileError
)
for /r %SHADER_DIR% %%f in (*.frag) do (
    "%GLSLC%" %%f -I %INCLUDE_DIR% -o %SHADER_DIR%\bin\%%~nf.spv || goto :CompileError
) 
for /r %SHADER_DIR% %%f in (*.comp) do (
    "%GLSLC%" %%f -I %INCLUDE_DIR% -o %SHADER_DIR%\bin\%%~nf.spv || goto :CompileError
) 
 
echo Shader compilation complete. 
goto :EOF
 
:CompileError
echo(
echo ************************************************************
echo *  Shader compilation FAILED. Scroll up for diagnostics.  *
echo ************************************************************
pause
exit /b 1
