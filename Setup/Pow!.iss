; Application name
#define   Name       "Pow!"
; Application version
#define   Version    "3.01a"
; Development company
#define   Publisher  "Pow! Team"
; Website of the developer's company
#define   URL        "https://github.com/Oleg-N-Cher/POW"
; Executable module name
#define   ExeName    "Pow.exe"

[Setup]
; The unique identifier of the application, 
;generated via Tools -> Generate GUID
AppId={{271EB057-3B5F-4017-BD63-C741AEEC9C64}

; Other information displayed during installation
AppName={#Name}
AppVersion={#Version}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
AppUpdatesURL={#URL}

; Default installation path
DefaultDirName={commonpf}\{#Name}
; Group name in the Start menu
DefaultGroupName=Pow! (32-Bit)

; The directory where the assembled setup will be
;written and the name of the executable file
OutputDir=.
OutputBaseFileName=Pow32_301a

; Файл иконки
SetupIconFile=Pow!.ico

; Параметры сжатия
Compression=lzma
SolidCompression=yes

[Languages]
Name: "English"  ; MessagesFile: "compiler:Default.isl";             LicenseFile: "License\En.txt"
Name: "German"   ; MessagesFile: "compiler:Languages\German.isl";    LicenseFile: "License\De.txt"
Name: "Russian"  ; MessagesFile: "compiler:Languages\Russian.isl";   LicenseFile: "License\Ru.txt"
Name: "Ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"; LicenseFile: "License\Ua.txt"
Name: "Hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"; LicenseFile: "License\Hu.txt"

[UninstallRun]
Filename: "{cmd}"; Parameters: "/C ""taskkill /im pow.exe /f /t"; Flags: runminimized; RunOnceId: KillPow

[Files]

; Исполняемый файл
Source: "Release\Pow.exe"; DestDir: "{app}"; Flags: ignoreversion
; Attached resources
Source: "Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; \
    GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Icons]

Name: "{group}\{#Name}"; Filename: "{app}\{#ExeName}"
Name: "{commondesktop}\{#Name}"; Filename: "{app}\{#ExeName}"; Tasks: desktopicon