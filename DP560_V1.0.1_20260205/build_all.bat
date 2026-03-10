@echo off
for /f "tokens=2 delims==" %%i in ('wmic os get localdatetime /value') do set datetime=%%i
set year=%datetime:~0,4%
set month=%datetime:~4,2%
set day=%datetime:~6,2%
set hour=%datetime:~8,2%
set minute=%datetime:~10,2%
set second=%datetime:~12,2%

echo Current: %year%%month%%day%_%hour%%minute%%second%
set CUR_TIME=%year%%month%%day%_%hour%%minute%%second%
:: -------------------------------------------
REM 设置git_Path
set GIT_PATH=D:\SWinstall\Git\Git\bin\bash.exe
::set FW_VER_LN=16
::set HAL_VER_LN=16
:: -------------------------------------------

REM 定义日志文件路径
set LOG_FILE="build_log_%CUR_TIME%.txt"

REM 调用 Git Bash 并执行脚本，将输出同时打印到终端和日志文件
call "%GIT_PATH%" --cd="%CD%\build" -c "./build.sh "all" 2>&1 | tee %LOG_FILE%"

@echo .
::echo  FW_ver: %hex2:~2%%hex1:~2%
::%GIT_PATH% -c "sed 's/[\#\"]/, HAL_ver: /g' ./src/cts_info.c | sed -n '%HAL_VER_LN%p' | cut -d, -f3"
@echo .
pause