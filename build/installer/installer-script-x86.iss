#define MyAppName "nest"
#define MyAppVersion "0.6.1"
#define MyAppPublisher "Davide Taffarello"
#define MyAppExeName "nest.exe"
#define MyAppAssocName "Nest File"
#define MyAppAssocExt ".nest"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

#include "addtopath.iss"

[Setup]
AppId={{90CC9EB3-3160-4D79-B767-3123B661A530}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} beta-{#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={userpf}\{#MyAppName}
DisableDirPage=yes
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.txt
; Remove the following line to run in administrative install mode (install for all users.)
PrivilegesRequired=lowest
OutputDir=.
OutputBaseFilename=nest-{#MyAppVersion}-x86-installer
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ChangesEnvironment=yes

[Tasks]
Name: envPath; Description: "Add to PATH variable" 

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\x86\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\x86\nest_libs\*"; DestDir: "{app}\nest_libs"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: "{#MyAppAssocExt}"; ValueData: ""

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Code]
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and WizardIsTaskSelected('envPath') then
    EnvAddPath(ExpandConstant('{app}'));
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
    EnvRemovePath(ExpandConstant('{app}'));
end;