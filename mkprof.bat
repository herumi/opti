@echo off

if "%1" == "" goto err
if "%2" == "" goto err

set bit=%FrameworkDir:~-2%
set pg="%ProgramFiles(x86)%
set prof_path=

if %bit% == 64 (
	set pg="c:/Program Files
) else (
	set bit=32
)
echo bit: %bit%
echo prof_path: %prof_path%
echo pg: %pg%

if "%1" == "a" (
set msg=AMD : useCodeAnalyst
set prof=USE_CODEANALYST
set prof_path=%pg%/AMD/CodeAnalyst/API/
) else (
set msg=Intel : use VTune
set prof=USE_VTUNE
set prof_path=%pg%/Intel/VTune Amplifier XE/
)

rem can't define proflib in the previous if-then-else
if "%1" == "a" (
set proflib=%prof_path%lib/%bit%-bit
) else (
set proflib=%prof_path%lib%bit%
)

set profinc=%prof_path%include

echo path: %prof_path%
echo inc : %profinc%
echo lib : %proflib%

cl /EHsc /Ox /Zi %2 -I c:/prog/xbyak -I %profinc%" -I "c:/Program Files/AMD/CodeAnalyst/API/include" /DUNICODE /D_UNICODE /D%prof% /link /LIBPATH:%proflib%" /DEBUG
echo cl /EHsc /Ox /Zi %2 -I c:/prog/xbyak -I %profinc%" -I "c:/Program Files/AMD/CodeAnalyst/API/include" /DUNICODE /D_UNICODE /D%prof% /link /LIBPATH:%proflib%" /DEBUG

goto exit
:err
echo "mk [a|i] file[.cpp]"

:exit
