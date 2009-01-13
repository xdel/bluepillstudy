:Step 1. Set command alias 
doskey up=cd /d ..
doskey up2=cd /d ..\..
doskey n=notepad $*
doskey buildfw=%ENLISTMENT_PROJ_ROOT%\src\Framework\build_code.cmd
doskey cleanfw=%ENLISTMENT_PROJ_ROOT%\src\Framework\clean.cmd
doskey start =explorer $*

:Step 2. Set MadDog Framework enviroments
set FRAMEWORK_NAME=Framework
set FRAMEWORK_SRC_ROOT=%ENLISTMENT_PROJ_ROOT%\src\%FRAMEWORK_NAME%
set FRAMEWORK_BIN_ROOT=%ENLISTMENT_PROJ_ROOT%\bin\%FRAMEWORK_NAME%
set FRAMEWORK_LOG_ROOT=%FRAMEWORK_BIN_ROOT%\log
