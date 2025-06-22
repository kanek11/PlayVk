@echo off
setlocal enabledelayedexpansion

rem === config ==========================================================
set "SHADER_DIR=shaders"
set "OUT_DIR=%SHADER_DIR%\bin"
set "DXC_FLAGS=-spirv -fspv-target-env=vulkan1.3"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

rem === VS / PS / CS loop =====================================
for /r "%SHADER_DIR%" %%f in (*.hlsl) do (
    rem ---------- Vertex Shader ----------
    dxc -T vs_6_0 -E VSMain "%%f" %DXC_FLAGS% ^
        -Fo "%OUT_DIR%\%%~nf_VS.spv"  || goto :CompileError

    rem ---------- Pixel(Fragment) Shader ----------
    dxc -T ps_6_0 -E PSMain "%%f" %DXC_FLAGS% ^
        -Fo "%OUT_DIR%\%%~nf_PS.spv" || goto :CompileError 

       rem ---------- Compute Shader ----------::dxc -T cs_6_0 -E CSMain "%%f" %DXC_FLAGS% ^ -Fo "%OUT_DIR%\%%~nf_CS.spv" || goto :CompileError


)

 
echo HLSL ? SPIR-V compilation succeeded.
goto :EOF

:CompileError
echo(
echo ************************************************************
echo *  Shader compilation FAILED. Scroll up for diagnostics.  *
echo ************************************************************
pause
exit /b 1
