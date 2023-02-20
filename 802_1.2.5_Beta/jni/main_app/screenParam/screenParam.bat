
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  Copyright: www.ibeelink.com 
::  author: aiden.xiang@ibeelink.com
::  data:2021.06.05
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set "DEL=%%a"
)
set GREEN_FONT=0A
set RED_FONT=0C
set YELLOW_FONT=0E

call :ColorText %YELLOW_FONT% " ----------------please select your Develop Environment---------------- "
echo 1: Sigmastar-V30(default) 
set select_dev=1
set /p select_dev=
if "%select_dev%"=="1" (
    set SELECT_ENV=TAKOYAKI_DLC00V030
)

call :ColorText %YELLOW_FONT% " ----------------please select your SceenParams---------------- "
echo 1: 15_4_1280x800(default) 2: 18_5_1366x768  3: 19_0_1440x900  4: 21_5_1920x1080  5: 1280x1024
set select_screen=1
set /p select_screen=
if "%select_screen%"=="1" (
    set SELECT_PARAMS=15_4_1280x800
    set SELECT_UI=player1280x800.ftu
)
if "%select_screen%"=="2" (
    set SELECT_PARAMS=18_5_1366x768
    set SELECT_UI=player1366x768.ftu
)
if "%select_screen%"=="3" (
    set SELECT_PARAMS=19_0_1440x900
    set SELECT_UI=player1440x900.ftu
)
if "%select_screen%"=="4" (
    set SELECT_PARAMS=21_5_1920x1080
    set SELECT_UI=player1920x1080.ftu
)
if "%select_screen%"=="5" (
    set SELECT_PARAMS=1280x1024
    set SELECT_UI=player1280x1024.ftu
)
if "%select_screen%"=="6" (
    set SELECT_PARAMS=1024x600
    set SELECT_UI=player1024x600.ftu
)
REM 专为sigmastar设置的环境变量
set COMPILE_SERVER=192.168.9.114
set USER=root
set COMPILE_PATH=/home/Jiajun/Sigmastar/%SELECT_ENV%
set LOG_MSG_1="please contact aiden.xiang@ibeelink.com"
set PROJECT_PATH=%cd%

set UPLOAD_SRC1=%SELECT_PARAMS%\fbdev.ini
set UPLOAD_SRC2=%SELECT_PARAMS%\zkgui
set UPLOAD_SRC3=%SELECT_PARAMS%\ota.sh
set UPLOAD_DEC1=%COMPILE_PATH%/sdk/verify/application/zk_full_security/res
set UPLOAD_DEC2=%COMPILE_PATH%/sdk/verify/application/zk_full_security/bin
set UPLOAD_DEC3=%COMPILE_PATH%/sdk/verify/application/zk_full_security/res

set UPLOAD_ROOT=%PROJECT_PATH%\flythingsUI

@REM subst W: %UPLOAD_ROOT%

echo.
call :ColorText %YELLOW_FONT% " ----------------start to check server---------------- "
ping -n 1 %COMPILE_SERVER%
if not %errorlevel%==0 (
  call:failed_log
)

call :ColorText %YELLOW_FONT% "----------------start to uploade----------------"

scp -r %UPLOAD_SRC1% %USER%@%COMPILE_SERVER%:%UPLOAD_DEC1%
if not %errorlevel%==0 (
  call:failed_log
)
scp -r %UPLOAD_SRC2% %USER%@%COMPILE_SERVER%:%UPLOAD_DEC2%
if not %errorlevel%==0 (
  call:failed_log
)
scp -r %UPLOAD_SRC3% %USER%@%COMPILE_SERVER%:%UPLOAD_DEC3%
if not %errorlevel%==0 (
  call:failed_log
)

call :ColorText %YELLOW_FONT% "----------------start to copy flythings UI----------------"
del W:\lib\libzkgui.so
del W:\ui\playerSelect.ftu
@REM del W:\ui\def_ad_img.png

del ..\..\..\ui\playerSelect.ftu
copy %SELECT_PARAMS%\%SELECT_UI% ..\..\..\ui\playerSelect.ftu
if not %errorlevel%==0 (
  call:failed_log
)

call :ColorText %YELLOW_FONT% "----------------start to make pack_files----------------"

call:success_log
:ColorText
echo off
<nul set /p ".=%DEL%" > "%~2"
findstr /v /a:%1 /R "^$" "%~2" nul
del "%~2" > nul 2>&1
echo.
goto :eof

:failed_log
echo.
echo ==============================================
call :ColorText %RED_FONT% "Sorry, auto_build failed !"
call :ColorText %RED_FONT% "start to clean project"
call :ColorText %YELLOW_FONT% %LOG_MSG_1%
echo ==============================================
echo.
pause
exit
goto:eof

:success_log
echo.
echo ==============================================
call :ColorText %GREEN_FONT% "Congratulations, auto_build success !"
echo ==============================================
echo.
pause
exit
goto:eof

:check_dir
if exist %~1 (		
		echo.
	) else (
		echo "creat the dir %~1"		
		md "%~1"
	)
goto:eof


@REM mkdir -p /mnt/xiangwei && mount -t nfs -o nolock 192.168.137.1:/w /mnt/xiangwei && source /mnt/xiangwei/copy.sh
