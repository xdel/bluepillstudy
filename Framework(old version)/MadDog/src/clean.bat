::@echo off
::del /Q /F /S *.obj
::del /Q /F /S *.mac
::rmdir /Q /S objchk_wxp_*
for /d %%i in (*) do (
	IF exist "%%i\objchk_wxp_*" (
		del /s /q /f  "%%i\objchk_wxp_*\*.obj" 
		del /s /q /f  "%%i\objchk_wxp_*\*.mac" 
		::rd /s /q "%%a" 
	::)
::)
::@echo on


