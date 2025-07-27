@echo off
setlocal enabledelayedexpansion

rem === CONFIG ==========================================================
set "SHADER_DIR=shaders"
set "OUT_DIR=%SHADER_DIR%\bin"
set "DXC_PATH=dxc"  rem change to the path of your DXC executable if needed
rem set "DXC_PATH="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\dxc.exe""

rem DXIL target flags (DXIL)
set "DXC_FLAGS=-Zi -Qembed_debug -O3 -WX"

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

rem === main loop =====================================
for /r "%SHADER_DIR%" %%f in (*.hlsl) do (

    rem === VSMai,PSMain,CSMain ===
    for %%e in (VSMain PSMain CSMain) do (
        findstr /i /c:"%%e" "%%f" >nul
        if !errorlevel! == 0 (
            set "ENTRY=%%e"
            if "%%e"=="VSMain" set "TARGET=vs_6_6"
            if "%%e"=="PSMain" set "TARGET=ps_6_6"
            if "%%e"=="CSMain" set "TARGET=cs_6_6"
 
            if "%%e"=="VSMain" set "AFFIX=VS"
            if "%%e"=="PSMain" set "AFFIX=PS"
            if "%%e"=="CSMain" set "AFFIX=CS"

            echo Compiling %%f with entry !ENTRY! and target !TARGET!

            %DXC_PATH% -T !TARGET! -E !ENTRY! "%%f" %DXC_FLAGS% ^
                -Fo "%OUT_DIR%\%%~nf.cso" ^
                -Fc "%OUT_DIR%\%%~nf_disasm.txt" || goto :CompileError
rem ===                 -Fo "%OUT_DIR%\%%~nf_!AFFIX!.cso" ^
rem ===                 -Fc "%OUT_DIR%\%%~nf_!AFFIX!_disasm.txt" || goto :CompileError
        )
    )
)

echo ============================================================
echo Shader compilation to DXIL succeeded.
echo Output path: %OUT_DIR%
echo ============================================================

if "%cmdcmdline:~0,1%"=="" (
    goto :EOF
) else (
    pause
    goto :EOF
)

:CompileError
echo(
echo ************************************************************
echo *  Shader compilation FAILED. Scroll up for diagnostics.  *
echo ************************************************************
pause
exit /b 1