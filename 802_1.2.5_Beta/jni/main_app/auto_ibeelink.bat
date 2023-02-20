
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  Copyright: www.ibeelink.com 
::  author: aiden.xiang@ibeelink.com
::  data:2021.06.02
::  main_app.h版本号--->auto_ibeelink.bat版本号
::  --->screenParam.bat--->编译UI--->编译和打包
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set "DEL=%%a"
)
set GREEN_FONT=0A
set RED_FONT=0C
set YELLOW_FONT=0E

set OUTPUT_PAC=outputPac
set OUTPUT_OTA=outputOta
set OTA_NAME=SStarOta.bin.gz_K.1.2.8

call :ColorText %YELLOW_FONT% " ----------------please select your Develop Environment---------------- "
echo 1: Sigmastar-V30(default) 
set select_dev=1
set /p select_dev=
if "%select_dev%"=="1" (
    set SELECT_ENV=TAKOYAKI_DLC00V030
)

call :ColorText %YELLOW_FONT% " ----------------please select your operation---------------- "
echo 1: Compiler(default)   2: Pack  3: Update
set select_opt=1
set /p select_opt=

set PROJECT_PATH=%cd%
REM 专为sigmastar设置的环境变量
set COMPILE_SERVER=192.168.9.114
set COMPILE_PATH=/home/Jiajun/Sigmastar/%SELECT_ENV%
set USER=root
set UPLOAD_ROOT=W:
set UPLOAD_SRC1=%UPLOAD_ROOT%\ui
set UPLOAD_SRC2=%UPLOAD_ROOT%\lib\libzkgui.so
set UPLOAD_DEC=%COMPILE_PATH%/sdk/verify/application/workfile
set SDK_DEC=%COMPILE_PATH%/sdk/verify/application/zk_full_security
set DOWNLOAD_DEC=%COMPILE_PATH%/project/image/output/images
set OTA_IMAGE_DEC=%DOWNLOAD_DEC%/SStarOta.bin.gz

set LINUX_CMD_1="cd %COMPILE_PATH% ; source /etc/profile ; cd %SDK_DEC% ;source copy.sh && cd %COMPILE_PATH% && source start.sh"
set LINUX_CMD_2="cd %COMPILE_PATH% ; source /etc/profile ; cd %COMPILE_PATH%/project && source auto_ibeelink_pack.sh"
set LINUX_CMD_3="cd %COMPILE_PATH% ; source /etc/profile ; cd %SDK_DEC% ;source copy.sh "
set LOG_MSG_1="please contact aiden.xiang@ibeelink.com"

echo.
call :ColorText %YELLOW_FONT% " ----------------start to check server---------------- "
ping -n 1 %COMPILE_SERVER%
if not %errorlevel%==0 (
  call:failed_log
)


if "%select_opt%"=="1" (

  call :ColorText %YELLOW_FONT% "----------------start to uploade----------------"
  scp -r %UPLOAD_SRC1% %USER%@%COMPILE_SERVER%:%UPLOAD_DEC%
  if not %errorlevel%==0 (
    call:failed_log
  )
  scp -r %UPLOAD_SRC2% %USER%@%COMPILE_SERVER%:%UPLOAD_DEC%
  if not %errorlevel%==0 (
    call:failed_log
  )


  call :ColorText %YELLOW_FONT% "----------------start to makefile----------------"
  ssh %USER%@%COMPILE_SERVER% %LINUX_CMD_1%
  if not %errorlevel%==0 (
    call:failed_log
  )

  del %PROJECT_PATH%\%OUTPUT_PAC% /q
  call :ColorText %YELLOW_FONT% "----------------start to copy output----------------"
  scp -r %USER%@%COMPILE_SERVER%:%DOWNLOAD_DEC%/* %PROJECT_PATH%\%OUTPUT_PAC%
  if not %errorlevel%==0 (
    call:failed_log
  )

)

if "%select_opt%"=="2" (

  call :ColorText %YELLOW_FONT% "----------------start to pack----------------"
  ssh %USER%@%COMPILE_SERVER% %LINUX_CMD_2%
  if not %errorlevel%==0 (
    call:failed_log
  )

  del %PROJECT_PATH%\%OUTPUT_OTA% /q
  call :ColorText %YELLOW_FONT% "----------------start to copy output----------------"
  scp -r %USER%@%COMPILE_SERVER%:%OTA_IMAGE_DEC% %PROJECT_PATH%\%OUTPUT_OTA%\%OTA_NAME%
  if not %errorlevel%==0 (
    call:failed_log
  )
  
)

if "%select_opt%"=="3" (

  call :ColorText %YELLOW_FONT% "----------------start to update Environment----------------"
  scp -r %UPLOAD_SRC1% %USER%@%COMPILE_SERVER%:%UPLOAD_DEC%
  if not %errorlevel%==0 (
    call:failed_log
  )
  scp -r %UPLOAD_SRC2% %USER%@%COMPILE_SERVER%:%UPLOAD_DEC%
  if not %errorlevel%==0 (
    call:failed_log
  )
  call :ColorText %YELLOW_FONT% "----------------start to makefile----------------"
  ssh %USER%@%COMPILE_SERVER% %LINUX_CMD_3%
  if not %errorlevel%==0 (
    call:failed_log
  )
)

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