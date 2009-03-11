@echo off

@rem Step 1. Record from where enters this directory
pushd %SAMPLE_OVERSHADOW_SRC_ROOT%\ContextIdentificationModule

@rem Step 2. Build parts
call build_code.bat

@rem Step 3. Return to the origin directory.
popd
@echo on

