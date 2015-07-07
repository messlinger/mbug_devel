; dpinst/dpscat driver installer example. 
; By Travis Robinson
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
; IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
; TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
; PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TRAVIS LEE ROBINSON
; BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
; THE POSSIBILITY OF SUCH DAMAGE.
;
#define DEVICE_DESCRIPTION  "Dual Benchmark Device"
#define INF_PROVIDER        "Travis Robinson"
#define PROVIDER_URL        "http://sourceforge.net/projects/libusbk"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{25787881-B25E-45F0-AB3F-EA71B11B311A}
AppName={#DEVICE_DESCRIPTION} Driver
AppVersion=3.0.5.2
;AppVerName=My Program 1.5
AppPublisher={#INF_PROVIDER}
AppPublisherURL={#PROVIDER_URL}
AppSupportURL={#PROVIDER_URL}
AppUpdatesURL={#PROVIDER_URL}
CreateAppDir=no
OutputBaseFilename=InstallDriver
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin
; requires WinXP, or higher (x86 or amd64 only)
MinVersion=0, 5.1.2600
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=x64

; Disable all wizard pages
DisableDirPage=yes
;DisableFinishedPage=yes
DisableProgramGroupPage=yes
DisableReadyMemo=yes
DisableReadyPage=yes
DisableStartupPrompt=yes
DisableWelcomePage=no
Uninstallable=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Code]
var
  InstallerProgressPage: TOutputProgressWizardPage;
  InstallerSuccess : Boolean;
  InstallerResultCode: Integer;
  
function IsX64: Boolean;
begin
  Result := Is64BitInstallMode and (ProcessorArchitecture = paX64);
end;

function IsI64: Boolean;
begin
  Result := Is64BitInstallMode and (ProcessorArchitecture = paIA64);
end;

function IsX86: Boolean;
begin
  Result := not IsX64 and not IsI64;
end;

function Is64: Boolean;
begin
  Result := IsX64 or IsI64;
end;

procedure InitializeWizard();
begin
  InstallerProgressPage := CreateOutputProgressPage('Installing device driver(s)','Installation may take several minutes to complete..');
end;

procedure LaunchDpInst(CmdParams: string);
var
  ResultCode: Integer;
  LogText:String;
  
begin
  InstallerSuccess:=false;
  InstallerResultCode:=$80000000;
  WizardForm.StatusLabel.Caption:='Installing driver..';
  InstallerProgressPage.Show();
  InstallerProgressPage.SetProgress(50,100);

  if Exec(ExpandConstant('{tmp}\dpinst.exe'), CmdParams, ExpandConstant('{tmp}'), SW_SHOW, ewWaitUntilTerminated, ResultCode) then
  begin
    // Store the dpinst result code in public var 'InstallerResultCode'
    // http://msdn.microsoft.com/en-us/library/windows/hardware/ff544790%28v=vs.85%29.aspx
    InstallerResultCode:=ResultCode;
    if ResultCode and $80000000 = $80000000 then
    begin
      LogText:='Installing driver failed. Error = ' + IntToStr(ResultCode);
      Log(LogText);
      WizardForm.StatusLabel.Caption:=LogText;
    end else begin
      InstallerSuccess:=true;
      InstallerProgressPage.SetProgress(100,100);
    end;
  end else begin
     InstallerSuccess:=false;
     LogText:='dpinst not found.';
     Log(LogText);
  end;
  WizardForm.StatusLabel.Caption:='Driver install complete.';
  InstallerProgressPage.Hide();
  InstallerResultCode:=ResultCode;
end;

function GetCustomSetupExitCode: Integer;
begin
  Result := 0;
  if not InstallerSuccess then
    Result:=InstallerResultCode;
end;

[Files]
Source: ".\amd64\*"; DestDir: "{tmp}\amd64"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\x86\*"; DestDir: "{tmp}\x86"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\*.inf"; DestDir: "{tmp}"; Flags: ignoreversion
Source: ".\dpscat.exe"; DestDir: "{tmp}"; Flags: ignoreversion;
Source: ".\dpinst32.exe"; DestDir: "{tmp}"; DestName: "dpinst.exe"; Flags: ignoreversion; Check: IsX86;
Source: ".\dpinst64.exe"; DestDir: "{tmp}"; DestName: "dpinst.exe"; Flags: ignoreversion; Check: IsX64;

[Run]
Filename: "{tmp}\dpscat.exe"; WorkingDir:"{tmp}"; Flags:runhidden; StatusMsg:"Creating catalog..";  AfterInstall: LaunchDpInst('/A /F /SW');

