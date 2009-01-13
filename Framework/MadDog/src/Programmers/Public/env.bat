:Step 1. Set command alias 
doskey up=cd /d ..
doskey up2=cd /d ..\..
doskey n=notepad $*
doskey buildfw=%ENLISTMENT_PROJ_ROOT%\src\Framework\build_code.cmd
doskey cleanfw=%ENLISTMENT_PROJ_ROOT%\src\Framework\clean.cmd
doskey start =explorer $*