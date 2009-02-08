@echo off

@rem Step 1. Record from where enters this directory
set _fromDir=%CD%
cd /d %FRAMEWORK_SRC_ROOT%

@rem Step 2. Build the Project
build -czgw -jpath %FRAMEWORK_LOG_ROOT%

@rem Step 3. Copy headers to the the output folder
@echo.
@echo Copy header files
copy /Y .\common\VTCore.h %FRAMEWORK_HEADERS_ROOT%
copy /Y .\common\VTCoreDebugger.h %FRAMEWORK_HEADERS_ROOT%
copy /Y .\common\VTCoreAPIs.h %FRAMEWORK_HEADERS_ROOT%
copy /Y .\common\VTCoreDefs.h %FRAMEWORK_HEADERS_ROOT%
copy /Y .\common\VTCoreTypes.h %FRAMEWORK_HEADERS_ROOT%
if not exist %FRAMEWORK_HEADERS_ROOT%\Vmx md %FRAMEWORK_HEADERS_ROOT%\Vmx
copy /Y .\vmx\vmcs.h %FRAMEWORK_HEADERS_ROOT%\Vmx

@rem Step 4. Return to the origin directory.
cd /d %_fromDir%
@echo on

