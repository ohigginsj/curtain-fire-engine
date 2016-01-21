@echo off

IF NOT EXIST bin mkdir bin

robocopy lib bin SDL2.dll /XC /XN /XO > NUL
robocopy lib bin SDL2_mixer.dll /XC /XN /XO > NUL

pushd bin

set Warnings= -WX -W4 -wd4100 -wd4201 -wd4127 -wd4189 -wd4055 -wd4090 -wd4456 -wd4457 -wd4312 -wd4244 -wd4996 -wd4577

:: Options
set DebugInfo= -Zi
set LinkMode= -MTd
set DisableExceptions= -EHa-
set Options= %DebugInfo% %LinkMode% %DisableExceptions%

set Includes= -I ..\include -I ..\include\freetype2

set GameSources= ..\src\game.cpp
set GameOutput= -OUT:cfe.dll
set PlatformSources= ..\src\main.cpp
set PlatformOutput= -Fewin32-platform.exe

set LinkerOptions= -incremental:no
set Libraries= ..\lib\freetype.lib ..\lib\SDL2.lib ..\lib\SDL2_mixer.lib user32.lib

cl %Warnings% %Options% %Includes% %GameSources% -LD /link %LinkerOptions% %Libraries% %GameOutput% -export:GameLoop
cl %Warnings% %Options% %Includes% %PlatformSources% %PlatformOutput% /link %LinkerOptions% %Libraries%

popd

pause