
:: dpscat 
:: This will create and install a self-signed certificate on the system.
:: Note that the private key for this certificate is immediately deleted
:: to prevent misuse.
:: You can get a list of the certificates intalled on your system by running
:: the certmgr.msc tool.
:: For additional information on this topic, please visit
:: https://github.com/pbatard/libwdi/wiki/Certification-Practice-Statement

:: dpinst
:: Windows Driver Package Installer
:: This tools can pre-install drivers that will be installed automatically 
:: when the respective devices are plugged in. The user still needs to
:: approve the installation.

dpscat.exe
dpinst.exe /C /LM /L 0x0409

pause
