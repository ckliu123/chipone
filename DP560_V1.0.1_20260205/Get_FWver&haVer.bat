@echo off
setlocal enabledelayedexpansion

set "info_file=src\cts_info.c"
set "fw_h_file=inc\cts_fw.h"

:: 提取AFE版本
if not exist "%info_file%" (
    echo ERROR: %info_file% not exist!!!
    goto fw_version
)

set "version="
:: 使用更精确的正则：匹配 #define AFE_VERSION "xxx" 格式，忽略注释
for /f "usebackq tokens=2,* delims=	 " %%a in (`findstr /r /c:"^[ 	]*#define[ 	][ 	]*AFE_VERSION[ 	][ 	]*" "%info_file%" 2^>nul`) do (
    set "raw=%%b"
    :: 去除前导空格，截取第一个token（处理注释情况）
    for /f "delims=	 " %%c in ("!raw!") do (
        set "version=%%~c"  :: %%~c 去除引号
        goto :got_version  :: 只取第一个匹配，避免覆盖
    )
)

:got_version
if not defined version (
    echo ERROR: %info_file% not found #define AFE_VERSION
    goto fw_version
)

:got_ver

:: 使用PowerShell提取数组元素（无需外部工具）
for %%p in (1 2) do (
    for /f "delims=" %%a in ('powershell -Command "(Get-Content '%fw_h_file%' -Raw) -match 'firmware_pid%%p\[\].*?=\s*\{([^}]+)\}'; ($matches[1] -split ',')[204].Trim()"') do set "hex%%p_1=%%a"
    for /f "delims=" %%a in ('powershell -Command "(Get-Content '%fw_h_file%' -Raw) -match 'firmware_pid%%p\[\].*?=\s*\{([^}]+)\}'; ($matches[1] -split ',')[205].Trim()"') do set "hex%%p_2=%%a"
)
echo.
echo  FW_ver_pid1: %hex1_2:~2%%hex1_1:~2%
echo  FW_ver_pid2: %hex2_2:~2%%hex2_1:~2%
echo  HAL_VER: %version%
echo.
pause