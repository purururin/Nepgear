# Nepgear

A DLL hijacking and hooking framework designed for Japanese visual novels (galgames). It masquerades as `winmm.dll` and loads alongside the game, providing locale emulation, font replacement, text conversion, file redirection, and engine-specific adaptations.

Supports both x86 and x64 architectures. Most visual novels are 32-bit, so the x86 version is typically used.

## Feature Overview

| Module | Description |
| --- | --- |
| DLL Proxying | Masquerades as `winmm.dll` and forwards exports to the system's original DLL; requires no modification to game files. |
| Locale Emulation | Integrates "Locale Emulator Plus" to restart the process with a Japanese locale. |
| Font Hooking | Handles font replacement, scaling, font weight, letter spacing, vertical metrics, and character set spoofing. |
| Text Conversion | Supports conversions between Japanese (legacy/variant) characters and Traditional/Simplified Chinese, using three distinct mapping tables. |
| Code Page Redirection | Modifies code pages at the `MultiByteToWideChar` / `WideCharToMultiByte` boundaries. |
| Engine Adaptation | Specialized font handling for 15 different engines (see table below). |
| Window Title | Offers three modes for modifying window titles; supports Unicode. |
| VFS & Redirection | Prioritizes loading files from external folders or `.chs` archive packages. |

### Supported Engines

BGI / Buriko, Artemis (including Legacy), KiriKiri / TVP, EntisGLS, Softpal, Escu:de, Mirai, Majiro, DxLib, TinkerBell, CatSystem2, Unity (Mono / IL2CPP / TextMeshPro), TyranoScript 5 / Electron, Ren'Py, and YU-RIS.

Engine-specific features are disabled by default; they must be enabled individually and are also subject to the master `[Font] Enable` switch. For implementation details specific to each engine, please refer to `Nepgear/hooks/internal/font/engines/*/README.md`. ## Building

Requires Visual Studio 2022 (v143 toolset) and the Windows SDK.

```bat
build.bat              Builds both architectures in Release mode
build.bat x86          Builds 32-bit only
build.bat x64          Builds 64-bit only
build.bat all rebuild  Performs a full rebuild
build.bat x86 debug    Builds in Debug configuration
```

`build_x86.bat` and `build_x64.bat` serve as shorthand entry points for individual architectures. Alternatively, you can open `Nepgear.sln` directly in Visual Studio; it contains all three projects (Nepgear, Packer, and Unpacker).

Build Outputs:

| Architecture | Output Directory |
| --- | --- |
| x86 | `Release\winmm.dll`, `Packer.exe`, `Unpacker.exe` |
| x64 | `x64\Release\winmm.dll`, `Packer.exe`, `Unpacker.exe` |

The Release build differs from standard projects in two ways. First, the generated `winmm.dll` is automatically compressed using UPX (reducing its size to approximately 39%). This compression process is integrated into the MSBuild workflow: a temporary copy is compressed and verified via `upx -t` before overwriting the final output file, ensuring that no corrupted files remain if the process is interrupted. Second, no `.pdb`, `.lib`, or `.exp` files are generated; the output directory contains only the executable files.

If you need PDB files for debugging, set the environment variable `NEPGEAR_SKIP_PACK=1` before building, or use the Debug configuration.

UPX 5.1.1 is included in the repository under `tools/upx/`. ## Deployment

Copy the following files into the directory containing the game's main executable (EXE):

1. `winmm.dll` — The architecture (x86/x64) must match that of the game process.
2. `Nepgear.ini` — Configuration file; **must be saved with UTF-16 LE encoding**. If this file is missing, the application will display an error message and exit.
3. Font file(s) — The filename must match the `FileName` specified in the `Nepgear.ini` file.

When the game launches, it will load the `winmm.dll` found in the same directory. Nepgear forwards the original function calls to the system's actual `winmm.dll`, ensuring that audio functionality remains unaffected.

If `[LocaleEmulator] Enable=1` is set, you must also place `LoaderDll_x86/x64.dll` and `LocaleEmulatorPlus_x86/x64.dll` into the designated redirection folder (by default, `Nepgear/`). These runtime files are not included in this repository.

## Configuration

For a complete, annotated configuration file, please refer to `Nepgear.ini` in the repository's root directory. Only the key settings are listed here. ### `[System]`

```ini
Enable=1              ; Master switch; 0 = disable all functions
ShowStartupPopup=1    ; Show a popup notification upon startup
```

### `[Font]`

```ini
Enable=1
FileName=galgame.ttf  ; Place in the game's root directory or a redirect folder; leave blank to use system fonts
FaceName=galgame      ; The 'FaceName' parameter for CreateFont; it is recommended to match the font's internal name
Charset=128           ; 128 = Shift-JIS, 134 = GB2312, 1 = Default

EnableFaceNameReplace=1
EnableCharsetReplace=0

EnableHeightScale=1
HeightScale=1.0       ; Supports decimal values; used to adjust font size if it appears too large or small after localization
EnableWidthScale=1
WidthScale=1.0
EnableWeight=1
Weight=0              ; 0 = Use the weight requested by the application; 400 = Normal; 700 = Bold
```

Character spacing, vertical metrics, and line spacing are controlled via: `EnableCharSpacing` / `CharSpacing`, `EnableVerticalMetrics` / `AscentPermille` / `DescentPermille`, and `EnableLineSpacing` / `LineSpacing`. Vertical metrics are adjusted by patching the `hhea` and `OS/2` tables using values ​​expressed as a per-mille (thousandths) of the `unitsPerEm`. Codepage Spoofing and Text Substitution:

```ini
EnableCodepageSpoof=0   ; Spoofs the codepage requested by the game (SpoofFromCharset) to the target codepage (SpoofToCharset)
SpoofFromCharset=128
SpoofToCharset=1

EnableTextSubstitution=0
TextSubstitutionMode=0  ; 0 = Legacy Japanese/variant characters; 1 = Traditional to Simplified Chinese; 2 = Simplified to Traditional Chinese
TextSubstitutionCodepage=932
```

### `[FontCompatibility]`

Some game engines are sensitive to certain GDI hooks. You can disable them individually here: `SkipDrawTextA`, `SkipFontDataQueries`, `SelectObjectTrackedOnly`, `HookCreateFontW`, `HookCreateFontIndirectW`, and `HookGetTextFace`. If fonts fail to apply or rendering issues occur, check this section first.

### `[FontEngines]`

Each engine has an `Enable*` toggle, followed by several specific settings. For example:

```ini
EnableBgiHook=0
BgiPatchGdiImports=1              ; Win32 only; has no effect on x64
BgiClearGlyphCacheOnSwitch=1

EnableArtemis=0
ArtemisPatchTables=1              ; Modifies font size, character spacing, and line spacing in `list_windows*.tbl` / IET text files
ArtemisRedirectFontFiles=1
ArtemisFontSize=0                 ; 0 = Keep the game's original value
ArtemisRubySize=-1                ; -1 = Keep the game's original value
```

The toggles for other engines (KiriKiri, EntisGLS, Softpal, Escu:de, Mirai, Majiro, DxLib, TinkerBell, Unity, Tyrano, Ren'Py, YU-RIS) follow the same naming convention. See `Nepgear.ini` for detailed comments on each item. CatSystem2 and YU-RIS do not have a dedicated master switch: for CatSystem2, features are auto-detected when `Enable=1` and `EnableFaceNameReplace=1` are set; for YU-RIS, they are activated alongside font replacement. YU-RIS also includes a `YurisAtlasCodepage` setting to control the codepage for graphics/atlases (where 0 = auto-detect based on resource structure).

### `[Window]`

```ini
Enable=1
TitleMode=2   ; 0=Synchronous (in-process)  1=WinEvent monitoring only  2=Hybrid (recommended)
Title=Custom Title
```

### `[FileRedirect]` and `[FileHook]`

```ini
Enable=1
Folder=Nepgear            ; Priority directory for file lookups
ArchiveFile=Nepgear.chs   ; Resource archive (for packed-file mode)

VFSMode=0                 ; 0=Read from physical disk  1=Read from memory
```

### `[LocaleEmulator]`

```ini
Enable=0
CodePage=932              ; 932=Shift-JIS  936=GBK
LocaleID=1041             ; 1041=Japan  2052=China
Charset=128
Timezone=Tokyo Standard Time
```

The appropriate `LoaderDll` and core DLLs are loaded based on the current process architecture (bitness). These are cached in the system's temporary directory during runtime, so they do not occupy space within the game's installation folder.

### `[CodePage]`

```ini
Enable=0
FromCodePage=932
ToCodePage=65001
```

Used for scenarios where an engine is hardcoded to decode UTF-8 scripts using CP932. Adjust this setting if character-set spoofing occurs too late (i.e., if the text has already become garbled before reaching the GDI layer).

### `[GLOBAL]`

For KiriKiri XP3 patches and RioShiina support: includes settings such as `EnableKrkrz` / `KrkrzPatchFile` and `EnableRioShiina` / `RioShiinaMode` (where 1 = translation redirection, 2 = unpacking). ### `[Debug]`

```ini
Enable=0                  ; Opens a CMD console to display hook logs
LogToFile=0               ; Outputs to Nepgear.log
EnableFontTrace=0         ; Enables fine-grained sampling for font hooks
FontSlowMs=50
FontTraceSampleLimit=0
```

If fonts fail to load, set `Enable=1` to check the logs; this will show which APIs are being intercepted and whether the fonts have loaded successfully.

## Auxiliary Tools

`Packer.exe`: Packages translation resources into a single `.chs` file. Arrange your files while maintaining the original directory structure, then drag the folder onto `Packer.exe`. A `.chs` file with the same name will be generated in the same directory; configure the `ArchiveFile` setting in the `.ini` file accordingly.

`Unpacker.exe`: Performs the reverse operation (unpacking) to verify the contents of a package.

Both are command-line tools; you can use them simply by dragging and dropping files or folders onto them.

## Directory Structure

```
Nepgear/            Main DLL project
dllmain.cpp       Entry point and hook installation sequence
hooks/
```
                                                                                                                                                    Subsystem aggregation units
internal/       `.cppinc` implementation fragments (not compiled independently)
font/engines/ 15 engine adapters
font/             Font table patching (`name` / `cmap` / `OS/2` / `hhea`)
Common/             Shared header files for Packer and Unpacker
Packer/ Unpacker/   Resource packing/unpacking tools
tools/upx/          UPX 5.1.1 (pinned to the repository)
```

For code organization conventions, please refer to `Nepgear/hooks/README.md`. Files with the `.cppinc` extension are implementation fragments; they share private state and raw API pointers with their corresponding `.cpp` files and cannot be compiled independently.
