
setlocal

set INCLUDE=..\..\..\include;..\..\..\src;%INCLUDE%
set LIB=..\..\lib;%LIB%

rc  mbug_2820_gui.rc
cl  mbug_2820_gui.c mbug_2820_gui.res /link mbug.lib user32.lib gdi32.lib comdlg32.lib

endlocal