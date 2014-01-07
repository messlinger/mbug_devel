:: Build the entire project (release build only)
:: Binaries go to bin/
:: Libraries got to lib/


rem @echo off
setlocal

:: ---- Environment paths ---- ::
set _src=%CD%/../src
set _inc=%CD%/../include
set _lib=%CD%/lib
set _bin=%CD%/bin
set _build=%CD%/build 

set _lusb=%CD%/ext/libusb-win32-bin-1.2.6.0
set _lusb_inc=%_lusb%/include
set _lusb_lib=%_lusb%/lib/msvc
:: ------------------------------------------------ :: 

set INCLUDE=%_inc%;%_lusb_inc%;%INCLUDE%
set LIB=%_lib%;%_lusb_lib%;%LIB%

pushd build

cl  %_src%/mbug.c       /c 
cl  %_src%/mbug_2110.c  /c
cl  %_src%/mbug_2151.c  /c
cl  %_src%/mbug_2165.c  /c
cl  %_src%/mbug_2810.c  /c 
cl  %_src%/mbug_2811.c  /c 
cl  %_src%/mbug_2818.c  /c
cl  %_src%/mbug_2820.c  /c


lib  /out:mbug.lib   mbug.obj  mbug_2110.obj  mbug_2151.obj  mbug_2165.obj ^
                     mbug_2810.obj  mbug_2811.obj  mbug_2818.obj  mbug_2820.obj ^
                     libusb.lib

cl /Felsmbug.exe     %_src%/lsmbug.c          /link mbug.lib
cl /Fembug_2151.exe  %_src%/mbug_2151_tool.c  /link mbug.lib
cl /Fembug_2165.exe  %_src%/mbug_2165_tool.c  /link mbug.lib
cl /Fembug_2810.exe  %_src%/mbug_2810_tool.c  /link mbug.lib
cl /Fembug_2811.exe  %_src%/mbug_2811_tool.c  /link mbug.lib
cl /Fembug_2818.exe  %_src%/mbug_2818_tool.c  /link mbug.lib
cl /Fembug_2820.exe  %_src%/mbug_2820_tool.c  /link mbug.lib
cl /Fembug_demo.exe  %_src%/mbug_demo.cpp  /EHsc  /link  mbug.lib

move /Y  *.lib  %_lib%
move /Y  *.exe  %_bin%

popd
endlocal
