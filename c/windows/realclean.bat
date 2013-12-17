:: Delete all intermediate and output files
:: (including binaries and libraries)

rem @echo off

call clean.bat

del /Q /S bin\*.*
del /Q /S lib\*.*
