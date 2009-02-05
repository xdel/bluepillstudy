@rem Step 1. Set command alias 
doskey up=cd /d ..
doskey up2=cd /d ..\..
doskey n=notepad $*
doskey buildAll=%ENLISTMENT_PROJ_ROOT%\src\buildAll.cmd
doskey cleanfw=%ENLISTMENT_PROJ_ROOT%\src\Framework\clean.cmd
doskey start =explorer $*

@rem Step 2. Set MadDog Framework environments
set FRAMEWORK_NAME=Framework
set FRAMEWORK_SRC_ROOT=%ENLISTMENT_PROJ_ROOT%\src\%FRAMEWORK_NAME%
set FRAMEWORK_BIN_ROOT=%ENLISTMENT_PROJ_ROOT%\bin\%FRAMEWORK_NAME%
set FRAMEWORK_LOG_ROOT=%FRAMEWORK_BIN_ROOT%\log

@rem Step 3. Set Framework Public Reference Location 
set FRAMEWORK_HEADERS_ROOT=%FRAMEWORK_BIN_ROOT%\inc
set FRAMEWORK_LIBRARY_ROOT=%FRAMEWORK_BIN_ROOT%\lib\*

@rem Step 4. Set Sample - Helloworld environments
set SAMPLE_HELLOWORLD_NAME=Helloworld
set SAMPLE_HELLOWORLD_SRC_ROOT=%ENLISTMENT_PROJ_ROOT%\src\Sample\%SAMPLE_HELLOWORLD_NAME%
set SAMPLE_HELLOWORLD_BIN_ROOT=%ENLISTMENT_PROJ_ROOT%\bin\Sample\%SAMPLE_HELLOWORLD_NAME%
set SAMPLE_HELLOWORLD_LOG_ROOT=%SAMPLE_HELLOWORLD_BIN_ROOT%\log

@rem Final. Create Important bin path
if not exist %FRAMEWORK_BIN_ROOT% mkdir %FRAMEWORK_BIN_ROOT%
if not exist %FRAMEWORK_LOG_ROOT% mkdir %FRAMEWORK_LOG_ROOT%
if not exist %FRAMEWORK_HEADERS_ROOT% mkdir %FRAMEWORK_HEADERS_ROOT%
if not exist %SAMPLE_HELLOWORLD_BIN_ROOT% mkdir %SAMPLE_HELLOWORLD_BIN_ROOT%
if not exist %SAMPLE_HELLOWORLD_LOG_ROOT% mkdir %SAMPLE_HELLOWORLD_LOG_ROOT%