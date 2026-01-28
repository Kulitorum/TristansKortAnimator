#include "setupvars.iss"
#include "version.iss"

[Setup]
ArchitecturesInstallIn64BitMode=x64
AppId={{8E2F4A1B-5C3D-4E6F-9A8B-7C1D2E3F4A5B}
AppName={#TargetProduct}
AppVersion={#VersionNumber}
AppVerName={#TargetProduct} v{#VersionNumber}
AppPublisher={#TargetCompany}
DefaultGroupName={#TargetCompany}
DefaultDirName={autopf}\{#TargetCompany}\{#TargetProduct}
OutputDir=output
OutputBaseFilename={#TargetName}_v{#VersionNumber}_win{#TargetArch}_installer
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
; SetupIconFile=..\resources\icon.ico  ; Uncomment if you add an icon
UninstallDisplayIcon={app}\{#TargetName}.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"

[CustomMessages]
english.installVcRuntime=Installing Visual C++ Runtime...
danish.installVcRuntime=Installerer Visual C++ Runtime...

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Icons]
Name: "{group}\{#TargetProduct}"; Filename: "{app}\{#TargetName}.exe"; WorkingDir: "{app}"; IconFilename: "{app}\{#TargetName}.exe"
Name: "{group}\Uninstall {#TargetProduct}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#TargetProduct}"; Filename: "{app}\{#TargetName}.exe"; Tasks: desktopicon; WorkingDir: "{app}"; IconFilename: "{app}\{#TargetName}.exe"

; Clear previous installation
[InstallDelete]
Type: filesandordirs; Name: "{app}\*"

[Files]
; MSVC Runtime redistributable
Source: "{#MsvcRedist_Dir}\vcredist_{#TargetArch}.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; Check: VCRedistNeedsInstall

; Main executable
Source: "{#AppBuildDir}\{#TargetName}.exe"; DestDir: "{app}"; Flags: ignoreversion

; Qt6 DLLs (deployed by windeployqt)
Source: "{#AppBuildDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; Qt plugins and QML modules (recursive)
Source: "{#AppBuildDir}\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\platforminputcontexts\*"; DestDir: "{app}\platforminputcontexts"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\qml\*"; DestDir: "{app}\qml"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\qmltooling\*"; DestDir: "{app}\qmltooling"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppBuildDir}\vectorimageformats\*"; DestDir: "{app}\vectorimageformats"; Flags: ignoreversion recursesubdirs createallsubdirs

; Data files (if any exist)
; Source: "{#AppBuildDir}\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

[Run]
; Install VC++ Runtime if needed
Filename: "{tmp}\vcredist_{#TargetArch}.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "{cm:installVcRuntime}"; Check: VCRedistNeedsInstall
; Launch application after install (optional)
Filename: "{app}\{#TargetName}.exe"; Description: "Launch {#TargetProduct}"; Flags: nowait postinstall skipifsilent unchecked

[Code]
function VCRedistNeedsInstall: Boolean;
var
  Version: String;
begin
  // Check if VC++ 2022 runtime is installed (14.x)
  // This is a simplified check - you may want to check specific version
  if RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64',
    'Version', Version) then
  begin
    Result := False;  // Already installed
  end
  else
    Result := True;   // Needs installation
end;
