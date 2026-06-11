@echo off
setlocal enabledelayedexpansion

:: ============================================================
:: compile_shaders.bat
:: Usage: compile_shaders.bat <compiler_exe> <input_dir> <output_dir>
::
:: Example (Visual Studio Pre-Build Event):
::   "$(SolutionDir)scripts\compile_shaders.bat" "C:\VulkanSDK\1.x.x\Bin\glslc.exe" "$(SolutionDir)shaders" "$(SolutionDir)shaders\compiled"
:: ============================================================

:: --- Argument validation ---
if "%~1"=="" (
    echo [ERROR] Argument 1 missing: path to SPIR-V compiler executable.
    exit /b 1
)
if "%~2"=="" (
    echo [ERROR] Argument 2 missing: input shader directory.
    exit /b 1
)
if "%~3"=="" (
    echo [ERROR] Argument 3 missing: output directory.
    exit /b 1
)

set COMPILER=%~1
set INPUT_DIR=%~2
set OUTPUT_DIR=%~3

:: --- Validate compiler exists ---
if not exist "%COMPILER%" (
    echo [ERROR] Compiler not found: "%COMPILER%"
    exit /b 1
)

:: --- Validate input directory exists ---
if not exist "%INPUT_DIR%" (
    echo [ERROR] Input directory not found: "%INPUT_DIR%"
    exit /b 1
)

:: --- Create output directory if it doesn't exist ---
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
    if errorlevel 1 (
        echo [ERROR] Failed to create output directory: "%OUTPUT_DIR%"
        exit /b 1
    )
    echo [INFO] Created output directory: "%OUTPUT_DIR%"
)

:: --- Compile shaders ---
set COMPILED=0
set FAILED=0

:: Supported GLSL shader extensions
for %%E in (vert frag comp geom tesc tese mesh task rgen rint rahit rchit rmiss rcall) do (
    for %%F in ("%INPUT_DIR%\*.%%E") do (
        if exist "%%F" (
            set INPUT_FILE=%%F
            set OUTPUT_FILE=%OUTPUT_DIR%\%%~nxF.spv

            echo [INFO] Compiling: %%~nxF  -^>  %%~nxF.spv
            "%COMPILER%" "%%F" -o "!OUTPUT_FILE!"

            if errorlevel 1 (
                echo [ERROR] Failed to compile: %%~nxF
                set /a FAILED+=1
            ) else (
                set /a COMPILED+=1
            )
        )
    )
)

:: --- Summary ---
echo.
echo [DONE] Compiled: %COMPILED% shader(s)   Failed: %FAILED%

if %FAILED% gtr 0 (
    echo [ERROR] One or more shaders failed to compile.
    exit /b 1
)

exit /b 0