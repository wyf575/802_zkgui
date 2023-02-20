:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  Copyright: www.ibeelink.com 
::  author: aiden.xiang@ibeelink.com
::  data:2021.03.26
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set "DEL=%%a"
)
set GREEN_FONT=0A
set RED_FONT=0C
set YELLOW_FONT=0E

git add ./*
call :ColorText %YELLOW_FONT% " ----------------[ please select your update Reason ]---------------- "

set /p inputReason=
git commit -m "%inputReason%"
call :ColorText %YELLOW_FONT% " ----------------[ updating now....., please wait.....  ]---------------- "
git push
call :ColorText %GREEN_FONT% " ----------------[ update success! ]---------------- "
:: 注意这里一定要调用goto :eof 表示代码运行结束 
:: 保证和隔离了下面的被调用函数
goto :eof


:: 下面是被调用的函数
:ColorText
echo off
<nul set /p ".=%DEL%" > "%~2"
findstr /v /a:%1 /R "^$" "%~2" nul
del "%~2" > nul 2>&1
echo.
goto :eof

