:: Delete all intermediate build files
:: Keep binaries and libraries

rem @echo off

del /Q /S msvc\*.pch
del /Q /S msvc\*.ncb
del /Q /S msvc\*.pdb
del /Q /S msvc\*.idb
del /Q /S msvc\*.ilk
del /Q /S msvc\*.exe
del /Q /S msvc\*.obj
del /Q /S msvc\*.user
del /Q /S msvc\*.opt
del /Q /S msvc\*.plg
for /R msvc\ /D %%X in (Debug Release) do rd /S /Q "%%X"

del /Q /S build\*.*
