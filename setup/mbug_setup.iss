; This is an Inno Setup source file for the MBUG host software installer.
; Inno Setup creates nice native looking Windows Installer packages.
; Standard install actions can be defined by simple declarations, but any
;   non-standard tasks have to be programmed in Delphi :(

#define Name "MBUG Software"
#define Version "1.4"
#define Publisher "Messlinger"

; Define as external to use the original source files. Else, a self contained
; setup executable will be built
#define ext "external" 

[Setup]
AppId={{53698414-8D7C-4A59-8563-17B9E37240A5}
AppName={#Name}
AppVersion={#Version}
AppVerName={#Name} {#Version}
AppPublisher={#Publisher}
DefaultDirName={pf}\messlinger\mbug
DefaultGroupName=Messlinger
SourceDir=..\..\..\
AllowNoIcons=yes
;InfoAfterFile=after.txt
OutputDir=.
OutputBaseFilename=mbug_setup
SetupIconFile=c\windows\icon\usb_16_32.ico
Compression=lzma
SolidCompression=yes
WizardImageFile=c\windows\setup\img\wiz_img.bmp
;DisableWelcomePage=yes
AllowRootDirectory=no
ChangesEnvironment=yes
FlatComponentsList=no
PrivilegesRequired=admin
SetupLogging=yes
ShowTasksTreeLines=yes
ArchitecturesInstallIn64BitMode=x64 ia64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
WelcomeLabel2=This will install the [name/ver] on your computer.%n%nPlease see the INSTALL.TXT file for additional information concerning the various installation options.%n%nIt is recommended that you close all other applications before continuing.
SelectDirDesc=Where should the [name] be installed?
SelectDirLabel3=Setup will install the [name] into the following folder.
PreparingDesc=Setup is preparing to install the [name] on your computer.
FinishedLabel=Setup has finished installing the [name] on your computer.
FinishedRestartLabel=To complete the installation of the [name], Setup must restart your computer. Would you like to restart now?
FinishedRestartMessage=To complete the installation of the [name], Setup must restart your computer.%n%nWould you like to restart now?

[Tasks]
Name: "desktopicon"; GroupDescription: "Additional icons"; Description: "{cm:CreateDesktopIcon}"; Flags: unchecked

Name: "driver"; GroupDescription: "Device drivers"; Description: "Install MBUG device drivers"; 
Name: "sign_driver"; GroupDescription: "Device drivers"; Description: "Sign MBUG device drivers (suppresses the security nag screens)"; 
Name: "rm_driver"; GroupDescription: "Device drivers"; Description: "Uninstall existing MBUG device drivers \
                    (only recommended if you experience driver problems)"; Flags: unchecked 
Name: "python2"; GroupDescription: "Additional modules"; Description: "Install MBUG Python 2 module"; 
#define py2_task_no  7
Name: "python3"; GroupDescription: "Additional modules"; Description: "Install MBUG Python 3 module"; 
#define py3_task_no  8
Name: "labview"; GroupDescription: "Additional modules"; Description: "Install Labview driver"; 
#define labview_task_no  9

Name: "addpath"; GroupDescription: "Environment"; Description: "Add MBUG tools folder to system PATH environment"
Name: "addinc"; GroupDescription: "Environment"; Description: "Add MBUG include folder to system INCLUDE environment"
Name: "addlib"; GroupDescription: "Environment"; Description: "Add MBUG library folder to system LIB environment"

[Types]
;If only one type declared: Combobox is omitted

[Components]

[Files]
Source: "{src}\*"; DestDir:  "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs {#ext};
Source: "{src}\c\windows\bin\*"; DestDir:  "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirs {#ext};
Source: "{src}\c\windows\lib\*.dll"; DestDir:  "{sys}"; Flags:  {#ext};
; Driver package installer dpinst.exe needs to be in the same folder as the inf file, but I prefer to store them in a separate subfolder
Tasks: driver rm_driver; Check: IsX86; Source: "{src}\driver\dpinst\dpinst_x86.exe"; DestName: "dpinst.exe"; DestDir: "{app}\driver"; \
                         Flags: ignoreversion deleteafterinstall {#ext};
Tasks: driver rm_driver; Check: IsX64; Source: "{src}\driver\dpinst\dpinst_amd64.exe"; DestName: "dpinst.exe"; DestDir: "{app}\driver"; \
                         Flags: ignoreversion deleteafterinstall {#ext};
Tasks: driver rm_driver; Check: IsI64; Source: "{src}\driver\dpinst\dpinst_ia64.exe"; DestName: "dpinst.exe"; DestDir: "{app}\driver"; \
                         Flags: ignoreversion deleteafterinstall {#ext};
; Additional modules
#define py_module  "mbug_dev"  
Tasks: python2; Source: "{src}\py\{#py_module}\*"; DestDir: {code:get_py2_path}\lib\site-packages\{#py_module}; \
                Flags: ignoreversion recursesubdirs createallsubdirs {#ext};
Tasks: python3; Source: "{src}\py\{#py_module}\*"; DestDir: {code:get_py3_path}\lib\site-packages\{#py_module}; \
                Flags: ignoreversion recursesubdirs createallsubdirs {#ext};
Tasks: labview; Source: "{src}\labview\*"; DestDir: {code:get_labview_instr_path}; \
                Flags: ignoreversion recursesubdirs createallsubdirs {#ext};

[Icons]
Tasks: desktopicon; Name: "{commondesktop}\MBUG Tools"; Filename: "{win}\explorer.exe"; Parameters: "/e,{app}\bin";
Name: "{group}\{cm:UninstallProgram,{#Name}}"; Filename: "{uninstallexe}"; IconFilename: "{src}\c\windows\icon\unins_16_32.ico" 
Name: "{group}\MBUG tools (terminal)"; Filename: "{cmd}"; Parameters: "/K title Terminal && cd /d ""{app}\bin"" && type mbug_tools.txt "
Name: "{group}\MBUG tools (folder)"; Filename: "{win}\explorer.exe"; Parameters: "/e,{app}\bin"

[Registry]
Root: HKLM; Subkey: "SOFTWARE\Messlinger\mbug"; ValueType: expandsz; ValueName: "install"; ValueData:"{app}";  Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Messlinger\mbug"; ValueType: expandsz; ValueName: "version"; ValueData:"{#Version}";  Flags: uninsdeletekey
; Add directory to system search path, but only if it is not already in there
Tasks: addpath; Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}\bin"; \
    Check: not in_env( 'PATH', ExpandConstant('{app}\bin') )
Tasks: addlib; Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "LIB"; ValueData: "{olddata};{app}\c\windows\lib"; \
    Check: not in_env( 'LIB', ExpandConstant('{app}\c\windows\lib') )
Tasks: addinc; Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "INCLUDE"; ValueData: "{olddata};{app}\c\include"; \
    Check: not in_env( 'INCLUDE', ExpandConstant('{app}\c\include') )

[Run]
Tasks: rm_driver; Filename: "{app}\driver\dpinst.exe"; Parameters: "/LM /U mbug.inf /L 0x0409"; StatusMsg: "Removing device drivers..."
Tasks: sign_driver; Filename: "{app}\driver\dpscat.exe"; StatusMsg: "Signing device drivers...";
Tasks: driver; Filename: "{app}\driver\dpinst.exe"; Parameters: "/LM /L 0x0409"; StatusMsg: "Installing device drivers...";

[InstallDelete]
; Legacy installers just copied all tools to systemroot. Better delete those.
Type: files; Name: "{win}\mbug_????.exe"
Type: files; Name: "{win}\mbug_????_gui.exe"
Type: files; Name: "{win}\lsmbug.exe"
Type: files; Name: "{win}\mbugdll.dll"


[Code]
//------------------------------------------------------------------------------
// Platform detection
function IsX64: Boolean;
begin Result := Is64BitInstallMode and (ProcessorArchitecture = paX64); end;

function IsI64: Boolean;
begin Result := Is64BitInstallMode and (ProcessorArchitecture = paIA64); end;

function IsX86: Boolean;
begin Result := not IsX64 and not IsI64; end;

function Is64: Boolean;
begin Result := IsX64 or IsI64; end;

//------------------------------------------------------------------------------
// Environmnet variable registry constants
const 
  env_root_key = HKEY_LOCAL_MACHINE;
  env_sub_key = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';
 
// Test if string is already containted in system environment variable
function in_env(env_var: string; s: string): boolean; 
var 
  value: string;
begin
  if not RegQueryStringValue( env_root_key, env_sub_key, env_var, value ) // Note: string is returned unexpanded (eg. %SYSTEMROOT%)
  then value := '';
  Result := Pos(';' + s + ';', ';' + value + ';') > 0;
end;

// Remove string from system environment variable
procedure remove_env(env_var: string; s: string);
var value: string;
begin
  if not RegQueryStringValue( env_root_key, env_sub_key, env_var, value) // Note: string is returned unexpanded (eg. %SYSTEMROOT%)
  then value := '';
  StringChangeEx( value, ';' + s, '', true );  // Dumb implementation: Try all possible semicolon positions
  StringChangeEx( value, s + ';', '', true );
  RegWriteStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', env_var, value);
end;  

//------------------------------------------------------------------------------

// Labview detection
const labview_root_key = HKEY_LOCAL_MACHINE;
const labview_sub_key = 'SOFTWARE\National Instruments\LabVIEW';

var labview_found: boolean;
var labview_instr_path: string;
                                  
function is_labview_found(): boolean;
begin result:=labview_found end;

function get_labview_instr_path(dummy: string): string;
begin result:=labview_instr_path end;

// Check for Labiew registry entries
procedure find_labview;
var 
  keys: TArrayOfString;
  i: integer;
  ver, cur_ver: String;
begin
  if RegQueryStringValue( labview_root_key, labview_sub_key+'\CurrentVersion', 'Version', cur_ver) then
    if RegGetSubkeyNames( labview_root_key, labview_sub_key, keys) then
      for i:=0 to GetArrayLength(keys)-1 do
        if RegQueryStringValue( labview_root_key, labview_sub_key+'\'+keys[i], 'Version', ver) then
          if (CompareStr( ver, cur_ver ) = 0) then 
            if RegQueryStringValue( labview_root_key, labview_sub_key+'\'+keys[i], 'LVPUBINSTRLIBDIR', labview_instr_path) then 
              labview_found := true;  
end;

//------------------------------------------------------------------------------

// Python detection
const py_root_key = HKEY_LOCAL_MACHINE;
const py_sub_key = 'SOFTWARE\Python\PythonCore';

var py2_found: boolean;
var py2_path: string;
var py3_found: boolean;
var py3_path: string;

function is_py2_found(): boolean;
begin result:=py2_found end;

function get_py2_path(dummy: string): string;
begin result:=py2_path end;

function is_py3_found(): boolean;
begin result:=py3_found end;

function get_py3_path(dummy: string): string;
begin result:=py3_path end;

// Check for python registry entries
procedure find_python;
var 
  keys: TArrayOfString;
  i: integer;
begin
  if RegGetSubkeyNames( py_root_key, py_sub_key, keys) then
    for i:=0 to GetArrayLength(keys)-1 do
    begin
      if CompareStr( '2.', Copy( keys[i], 0, 2 ) ) = 0 then 
        if RegQueryStringValue( py_root_key, py_sub_key+'\'+keys[i]+'\InstallPath', '', py2_path) then 
          py2_found := true;  
      if CompareStr( '3.', Copy( keys[i], 0, 2 ) ) = 0 then 
        if RegQueryStringValue( py_root_key, py_sub_key+'\'+keys[i]+'\InstallPath', '', py3_path) then 
          py3_found := true;  
    end;
end;

// If not found, disable the detect dependent tasks
procedure disable_tasks();
begin
  if not py2_found then
  begin
    Wizardform.TasksList.Checked[{#py2_task_no}] := false;
    Wizardform.TasksList.ItemEnabled[{#py2_task_no}] := false;
    Wizardform.TasksList.ItemCaption[{#py2_task_no}] :=  Wizardform.TasksList.ItemCaption[{#py2_task_no}] \
           + ' (Python 2 not detected)';
  end;
  if not py3_found then
  begin
    Wizardform.TasksList.Checked[{#py3_task_no}] := false;
    Wizardform.TasksList.ItemEnabled[{#py3_task_no}] := false;
    Wizardform.TasksList.ItemCaption[{#py3_task_no}] :=  Wizardform.TasksList.ItemCaption[{#py3_task_no}] \
           + ' (Python 3 not detected)';
  end;
  if not labview_found then
  begin
    Wizardform.TasksList.Checked[{#labview_task_no}] := false;
    Wizardform.TasksList.ItemEnabled[{#labview_task_no}] := false;
    Wizardform.TasksList.ItemCaption[{#labview_task_no}] :=  Wizardform.TasksList.ItemCaption[{#labview_task_no}] \
           + ' (Labview not detected)';
  end;
end;  



//------------------------------------------------------------------------------

// Uninstall event: Called at uninstall runtime (between the several steps)
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then  // now before the actual uninstall procedure
    remove_env( 'PATH', ExpandConstant('{app}\bin') );
    remove_env( 'LIB', ExpandConstant('{app}\c\windows\lib') );
    remove_env( 'INCLUDE', ExpandConstant('{app}\c\include') );
end;


// Install runtime event: Called before and after the actual installation
procedure CurStepChanged(CurStep: TSetupStep);
begin
end;


// Install runtime event: Called at setup program initialization
function InitializeSetup(): Boolean;
begin
    find_python;
    find_labview;
    result := true;
end;


// Install runtime event: Called when a Wizard page is shown
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectTasks then
    disable_tasks;
end;