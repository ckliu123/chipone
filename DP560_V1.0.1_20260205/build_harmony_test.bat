@echo off

setlocal
REM 设置H文件名称 & 目标目录
set "filename=cts_fw.h" 
set "Foldername=inc"
set "BinfolderName=bin"
@echo.
echo  Start BIN to cts_fw.h
REM bin --> cts_fw.h
python bin2h.py 
timeout /t 3 /nobreak >nul
@echo.
if not exist "%filename%" (
	echo file %filename% not exist
	goto end
)

if not exist "%Foldername%\" (
	echo Folder %Foldername% not exist
)

:: 移动H文件到目标目录
move "%filename%" "%Foldername%\"
if errorlevel 1 (
    echo move %filename% to %Foldername% Failed---
    goto end
) 

:: 移动BIN文件到目标目录
if not exist "%BinfolderName%\" (
	md "%BinfolderName%"
)

move "*.bin" "%BinfolderName%\"
if errorlevel 1 (
    echo move BIN-File to %BinfolderName% Failed---
    goto end
) 
:end
for /f "tokens=2 delims==" %%i in ('wmic os get localdatetime /value') do set datetime=%%i
set year=%datetime:~0,4%
set month=%datetime:~4,2%
set day=%datetime:~6,2%
set hour=%datetime:~8,2%
set minute=%datetime:~10,2%
set second=%datetime:~12,2%
::echo Current: %year%%month%%day%_%hour%%minute%%second%
set CUR_TIME=%year%%month%%day%_%hour%%minute%%second%
set "currentDir=%~dp0"

:: ------------------------
REM 设置git_Path
set GIT_PATH=D:\SWinstall\Git\Git\bin\bash.exe
set FW_VER_LN=16
set HAL_VER_LN=16
:: ------------------------

REM 定义日志文件
set LOG_FILE="build_log_%CUR_TIME%.txt"

@echo.
for /f "delims=" %%i in ('%GIT_PATH% -c "sed -n '%FW_VER_LN%p' ./inc/cts_fw.h | cut -d, -f13,14"') do set "fwversion=%%i"
for /f "tokens=1,2 delims=, " %%a in ("%fwversion%") do (
    set "hex1=%%a"
    set "hex2=%%b"
)	
echo  FW_ver: %hex2:~2%%hex1:~2%
%GIT_PATH% -c "sed 's/[\#\"]/, HAL_ver: /g' ./src/cts_info.c | sed -n '%HAL_VER_LN%p' | cut -d, -f3"
@echo.

:: 调用 Git Bash 并执行脚本，将输出同时打印到终端和日志文件
::call "%GIT_PATH%" --cd=%currentDir% -c "./harmony.sh" 2>&1 | tee "%LOG_FILE%"
call "%GIT_PATH%" --cd=%currentDir% -c "./build_harmony_test.sh"

@echo.
echo Build Finished. 
@echo.
echo  FW_ver: %hex2:~2%%hex1:~2%
%GIT_PATH% -c "sed 's/[\#\"]/, HAL_ver: /g' ./src/cts_info.c | sed -n '%HAL_VER_LN%p' | cut -d, -f3"
@echo.

endlocal
pause
