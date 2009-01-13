@echo off

:Step 1. Record from where enters this directory
set _fromDir=%CD%
cd /d %FRAMEWORK_SRC_ROOT%

:Step 2. Delete object files
del /Q /F /S *.obj
del /Q /F /S *.mac

:Step 3. Return to the origin directory.
cd /d %_fromDir%

@echo on


