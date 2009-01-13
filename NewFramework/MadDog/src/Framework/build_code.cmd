@echo off

:Step 1. Set global enviroments
set FRAMEWORK_NAME=Framework
set FRAMEWORK_SRC_ROOT=%ENLISTMENT_PROJ_ROOT%\src\%FRAMEWORK_NAME%
set FRAMEWORK_BIN_ROOT=%ENLISTMENT_PROJ_ROOT%\bin\%FRAMEWORK_NAME%
set FRAMEWORK_LOG_ROOT=%FRAMEWORK_BIN_ROOT%\log

:Step 2. Record from where enters this directory
set _fromDir=%CD%
cd /d %FRAMEWORK_SRC_ROOT%

:Step 3. Build the Project
build -czgw -jpath %FRAMEWORK_LOG_ROOT%

:Step 4. Return to the origin directory.
cd /d %_fromDir%
@echo on

