@rem Build All the Existing Projects

@echo.
@echo =======Build Framework Files=======
call %ENLISTMENT_PROJ_ROOT%\src\Framework\build_code.bat

@echo.
@echo =======Build Helloworld Sample =======
call %ENLISTMENT_PROJ_ROOT%\src\Sample\Helloworld\build_code.bat

@echo.
@echo =======Build Overshadow Sample =======
call %ENLISTMENT_PROJ_ROOT%\src\Sample\Overshadow\build_code.bat