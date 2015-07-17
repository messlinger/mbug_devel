# Microsoft Developer Studio Project File - Name="mbug_2810_gui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=mbug_2810_gui - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "mbug_2810_gui.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mbug_2810_gui.mak" CFG="mbug_2810_gui - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mbug_2810_gui - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "mbug_2810_gui - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mbug_2810_gui - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\include" /I "..\..\ext\libusb-win32-bin-1.2.6.0\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 nafxcw.lib libcmt.lib libusb.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"nafxcw.lib" /nodefaultlib:"libcmt.lib" /out:"Release/mbug_2810_gui_mfc.exe" /libpath:"..\..\lib" /libpath:"..\..\ext\libusb-win32-bin-1.2.6.0\lib\msvc"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy binary
PostBuild_Cmds=copy release\mbug_2810_gui_mfc.exe ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "mbug_2810_gui - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /I "..\..\ext\libusb-win32-bin-1.2.6.0\include" /I "..\..\ext\stdint" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libusb.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/mbug_2810_gui_mfc.exe" /pdbtype:sept /libpath:"..\..\lib" /libpath:"..\..\ext\libusb-win32-bin-1.2.6.0\lib\msvc"

!ENDIF 

# Begin Target

# Name "mbug_2810_gui - Win32 Release"
# Name "mbug_2810_gui - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\mbug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\mbug_2810.c
# End Source File
# Begin Source File

SOURCE=.\mbug_2810_gui.cpp
# End Source File
# Begin Source File

SOURCE=.\mbug_2810_gui.rc
# End Source File
# Begin Source File

SOURCE=.\mbug_2810_guiDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\mbug_2811.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\mbug_2820.c
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\mbug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\mbug_2810.h
# End Source File
# Begin Source File

SOURCE=.\mbug_2810_gui.h
# End Source File
# Begin Source File

SOURCE=.\mbug_2810_guiDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\icon\usb_16.ico
# End Source File
# Begin Source File

SOURCE=..\..\icon\usb_32.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
