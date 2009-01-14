@echo off
:Part 1. Set environment variables.
:TODO - This part should be moved to the environment later
set ENLISTMENT_PROJ_ROOT=%%ENLISTMENT_PROJ_ROOT%%
set RAZZLE_INDEX=%%RAZZLE_INDEX%%
set DDKHome=E:\WinDDK\6001.18001
set _NT_SYMBOL_PATH=%_NT_SYMBOL_PATH%;%ENLISTMENT_PROJ_ROOT%\bin\Framework\bin\i386
set path=%path%;%ENLISTMENT_PROJ_ROOT%\bin\Tools\

:Step 1.1 Call Public\env.bat to setup other env vars.
call %ENLISTMENT_PROJ_ROOT%\src\Programmers\Public\env.bat

:Step 2. Set command alias
doskey src=cd /d %ENLISTMENT_PROJ_ROOT%\src\Framework
doskey bin=cd /d %ENLISTMENT_PROJ_ROOT%\bin\Framework
doskey test=cd /d %ENLISTMENT_PROJ_ROOT%\test

:Step 3. Startup build env.
%windir%\System32\cmd.exe /k %DDKHome%\bin\setenv.bat %DDKHome% %1 %2

src
@echo on
