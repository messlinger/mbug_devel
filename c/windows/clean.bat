:: Delete all intermediate build files
:: Keep binaries and libraries

rem @echo off

set files=*.bsc *.tlb *.opt *.res *.plg *.pch *.ncb *.pdb ^
          *.idb *.ilk *.exe *.obj *.suo *.user *.manifest
for /R msvc\ %%X in (%files%) do del /Q /S "%%X"
for /R msvc\ /D %%X in (Debug Release) do rd /S /Q "%%X"
del /Q /S build\*.*
