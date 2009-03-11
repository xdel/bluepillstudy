@echo off
@rem Part 1. Set environment variables.
:TODO - This part should be moved to the environment later
set ENLISTMENT_PROJ_ROOT=%%ENLISTMENT_PROJ_ROOT%%
set RAZZLE_INDEX=%%RAZZLE_INDEX%%
set DDKHome=%%WINDDK_HOME%%
set CURRENT_USER=%%CURRENTUSER%%
set _NT_SYMBOL_PATH=%_NT_SYMBOL_PATH%;%ENLISTMENT_PROJ_ROOT%\bin\Framework\bin\i386
set path=%path%;%ENLISTMENT_PROJ_ROOT%\bin\Tools\

@rem Step 1.1 Call Public\env.bat to setup other env vars.
call %ENLISTMENT_PROJ_ROOT%\src\Programmers\Public\env.bat

@rem Step 1.2 Call User\env.bat to setup other env vars.
call %ENLISTMENT_PROJ_ROOT%\src\Programmers\%CURRENT_USER%\env.bat

@rem Step 2. Set command alias
doskey src=cd /d %ENLISTMENT_PROJ_ROOT%\src\Framework
doskey bin=cd /d %ENLISTMENT_PROJ_ROOT%\bin\Framework
doskey test=cd /d %ENLISTMENT_PROJ_ROOT%\test

@rem Step 3. Startup build env.
%windir%\System32\cmd.exe /k %DDKHome%\bin\setenv.bat %DDKHome% %1 %2

src
@echo on
