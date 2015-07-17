; CLW-Datei enthält Informationen für den MFC-Klassen-Assistenten

[General Info]
Version=1
LastClass=CMbug_2820_guiDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "mbug_2820_gui.h"

ClassCount=2
Class1=CMbug_2820_guiApp
Class2=CMbug_2820_guiDlg

ResourceCount=4
Resource2=IDR_MAINFRAME
Resource3=IDD_MBUG_2820_GUI_DIALOG
Resource4=IDD_MBUG_2820_GUI_DIALOG (Englisch (USA))

[CLS:CMbug_2820_guiApp]
Type=0
HeaderFile=mbug_2820_gui.h
ImplementationFile=mbug_2820_gui.cpp
Filter=N

[CLS:CMbug_2820_guiDlg]
Type=0
HeaderFile=mbug_2820_guiDlg.h
ImplementationFile=mbug_2820_guiDlg.cpp
Filter=D
LastObject=IDC_BUTTON_MORE
BaseClass=CDialog
VirtualFilter=dWC



[DLG:IDD_MBUG_2820_GUI_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CMbug_2820_guiDlg

[DLG:IDD_MBUG_2820_GUI_DIALOG (Englisch (USA))]
Type=1
Class=CMbug_2820_guiDlg
ControlCount=16
Control1=IDC_GROUP_TEMPERATURE,button,1342177287
Control2=IDC_LABEL_VALUE_TEMPERATURE,static,1342308364
Control3=IDC_LABEL_DEVICE,static,1342308352
Control4=IDC_LABEL_STATUS,static,1342308352
Control5=IDC_LABEL_VALUE_STATUS,static,1342308364
Control6=IDC_COMBO_DEVICE,combobox,1344339971
Control7=IDC_HEARTBEAT_T,static,1207959556
Control8=IDC_GROUP_HUMIDITY,button,1342177287
Control9=IDC_LABEL_VALUE_HUMIDITY,static,1342308364
Control10=IDC_HEARTBEAT_HUM,static,1207959556
Control11=IDC_EDIT_LOGFILE,edit,1350631552
Control12=IDC_BUTTON_LOGFILE,button,1342246720
Control13=IDC_BUTTON_START,button,1342242816
Control14=IDC_BUTTON_STOP,button,1342242816
Control15=IDC_GROUP_LOG,button,1342177287
Control16=IDC_BUTTON_MORE,button,1342246784

