:: Install binaries to system folder
:: %sytemRoot% (normally c:/windows) should be in the search path normally


copy bin\*.exe %systemRoot%
copy lib\*.dll %systemRoot%

pause
