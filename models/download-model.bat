@echo off
setlocal enabledelayedexpansion

REM Script to download a specific Whisper GGML model for Windows
REM Usage: download-model.bat [model_name]
REM Default model: base

REM Hugging Face repository
set "SRC=https://huggingface.co/ggerganov/whisper.cpp"
set "PFX=resolve/main/ggml"

REM Available models
set "MODELS=tiny tiny.en tiny-q5_1 tiny.en-q5_1 tiny-q8_0 base base.en base-q5_1 base.en-q5_1 base-q8_0 small small.en small-q5_1 small.en-q5_1 small-q8_0 medium medium.en medium-q5_0 medium.en-q5_0 medium-q8_0 large-v1 large-v2 large-v2-q5_0 large-v2-q8_0 large-v3 large-v3-q5_0 large-v3-turbo large-v3-turbo-q5_0 large-v3-turbo-q8_0"

REM Get model name from argument or use default
if "%~1"=="" (
    set "MODEL=base"
) else if "%~1"=="--help" (
    goto :show_help
) else if "%~1"=="-h" (
    goto :show_help
) else (
    set "MODEL=%~1"
)

REM Validate model
set "VALID=0"
for %%m in (%MODELS%) do (
    if "%%m"=="%MODEL%" set "VALID=1"
)

if "%VALID%"=="0" (
    echo Error: Invalid model '%MODEL%'
    echo.
    call :list_models
    exit /b 1
)

REM Get script directory
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

REM Check if model already exists
if exist "ggml-%MODEL%.bin" (
    echo Model 'ggml-%MODEL%.bin' already exists. Skipping download.
    exit /b 0
)

REM Download the model
echo Downloading ggml model '%MODEL%' from Hugging Face...
echo Source: %SRC%/%PFX%-%MODEL%.bin
echo Destination: %SCRIPT_DIR%ggml-%MODEL%.bin
echo.

REM Use PowerShell BITS transfer for reliable downloading
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "try { " ^
    "    Write-Host 'Starting download...' -ForegroundColor Green; " ^
    "    Import-Module BitsTransfer -ErrorAction Stop; " ^
    "    Start-BitsTransfer -Source '%SRC%/%PFX%-%MODEL%.bin' -Destination '%SCRIPT_DIR%ggml-%MODEL%.bin' -Description 'Downloading Whisper Model' -DisplayName 'Model: %MODEL%'; " ^
    "    if (Test-Path '%SCRIPT_DIR%ggml-%MODEL%.bin') { " ^
    "        $size = (Get-Item '%SCRIPT_DIR%ggml-%MODEL%.bin').Length / 1MB; " ^
    "        Write-Host ('Downloaded successfully! File size: {0:N2} MB' -f $size) -ForegroundColor Green; " ^
    "        exit 0; " ^
    "    } else { " ^
    "        Write-Host 'Download failed: File not created' -ForegroundColor Red; " ^
    "        exit 1; " ^
    "    } " ^
    "} catch { " ^
    "    Write-Host ('Error: ' + $_.Exception.Message) -ForegroundColor Red; " ^
    "    Write-Host 'Trying alternative download method...' -ForegroundColor Yellow; " ^
    "    try { " ^
    "        $ProgressPreference = 'SilentlyContinue'; " ^
    "        Invoke-WebRequest -Uri '%SRC%/%PFX%-%MODEL%.bin' -OutFile '%SCRIPT_DIR%ggml-%MODEL%.bin' -UseBasicParsing; " ^
    "        if (Test-Path '%SCRIPT_DIR%ggml-%MODEL%.bin') { " ^
    "            $size = (Get-Item '%SCRIPT_DIR%ggml-%MODEL%.bin').Length / 1MB; " ^
    "            Write-Host ('Downloaded successfully! File size: {0:N2} MB' -f $size) -ForegroundColor Green; " ^
    "            exit 0; " ^
    "        } " ^
    "    } catch { " ^
    "        Write-Host ('Alternative method also failed: ' + $_.Exception.Message) -ForegroundColor Red; " ^
    "        exit 1; " ^
    "    } " ^
    "}"

if %ERRORLEVEL% equ 0 (
    echo.
    echo ============================================
    echo Success! Model downloaded successfully.
    echo Location: %SCRIPT_DIR%ggml-%MODEL%.bin
    echo.
    echo Usage example:
    echo   rvtt.exe -m models\ggml-%MODEL%.bin
    echo ============================================
    echo.
    exit /b 0
) else (
    echo.
    echo ============================================
    echo Error: Failed to download model '%MODEL%'
    echo Please check your internet connection and try again.
    echo ============================================
    echo.
    exit /b 1
)

:show_help
echo Usage: %~nx0 [model_name]
echo.
echo Download a Whisper GGML model from Hugging Face.
echo If no model name is specified, 'base' will be downloaded.
call :list_models
exit /b 0

:list_models
echo.
echo Available models:
echo   Tiny models:    tiny, tiny.en, tiny-q5_1, tiny.en-q5_1, tiny-q8_0
echo   Base models:    base, base.en, base-q5_1, base.en-q5_1, base-q8_0
echo   Small models:   small, small.en, small-q5_1, small.en-q5_1, small-q8_0
echo   Medium models:  medium, medium.en, medium-q5_0, medium.en-q5_0, medium-q8_0
echo   Large models:   large-v1, large-v2, large-v2-q5_0, large-v2-q8_0
echo                   large-v3, large-v3-q5_0
echo   Turbo models:   large-v3-turbo, large-v3-turbo-q5_0, large-v3-turbo-q8_0
echo.
echo Note:
echo   .en = English-only models (faster, smaller)
echo   -q5_0/-q5_1/-q8_0 = Quantized models (smaller file size)
echo.
exit /b 0

