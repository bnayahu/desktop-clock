# Desktop Clock

A lightweight, customizable floating digital clock widget for Windows 10/11. Built with the Win32 API and cross-compiled on Linux with MinGW-w64.

![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue)
![Release](https://img.shields.io/github/v/release/bnayahu/desktop-clock)
![License](https://img.shields.io/badge/license-MIT-green)

Single executable, no installer, no runtime dependencies. Settings live in the registry, so the clock remembers how you left it.

## Features

- **Floating window** — transparent, draggable from anywhere, optionally always on top
- **Resizable** — drag any edge or corner; the font scales with the window
- **Time format** — 12-hour (with AM/PM) or 24-hour, with optional seconds
- **Appearance**
  - Custom text color via an RGB picker with live preview
  - Independent background and text opacity (0–255 each), so text stays readable over a near-invisible window
  - Seven fonts: Segoe UI, Arial, Calibri, Consolas, Courier New, Times New Roman, Verdana
  - Three font sizes: Small, Medium, Large
- **System tray** — minimize to tray with a right-click menu
- **Persistence** — all preferences, plus window position and size, saved to the Windows Registry
- **Optional startup** — launch automatically when Windows starts

## Install

Download **[DesktopClock.exe](https://github.com/bnayahu/desktop-clock/releases/latest/download/DesktopClock.exe)** from the [latest release](https://github.com/bnayahu/desktop-clock/releases/latest), put it anywhere, and double-click it. The clock appears centered on your primary monitor and adds a tray icon.

Requires Windows 10 or Windows 11 (x64).

> **First-run warning:** the binary is not code-signed, so Windows SmartScreen may block it. Choose **More info → Run anyway**. Each release publishes a SHA-256 checksum if you want to verify the download first:
>
> ```powershell
> Get-FileHash DesktopClock.exe -Algorithm SHA256
> ```

## Usage

| Action | How |
| --- | --- |
| Move the clock | Click and drag anywhere on it |
| Resize | Drag a window edge or corner |
| Show / hide | Double-click the tray icon |
| Settings | Right-click the tray icon → **Settings...** |
| Exit | Right-click the tray icon → **Exit** |

The tray menu also toggles **Always on Top** and **Start with Windows** directly, and has an **About** entry.

### Settings dialog

Everything is in one dialog with a live preview:

- **Time format** — 12-hour or 24-hour
- **Show seconds** — when on, the clock updates every second instead of every minute
- **Always on top** — keep the clock above other windows
- **Text color** — **Choose Color...** opens the picker; the swatch shows the current color
- **Background opacity** — 0 (invisible) to 255 (opaque); controls the window itself
- **Text opacity** — 0 to 255; controls the digits independently of the background
- **Font** and **Font size**
- **Start with Windows**

**Apply** commits changes without closing the dialog.

## Configuration

Settings are stored under `HKEY_CURRENT_USER\Software\DesktopClock`:

| Value | Contents |
| --- | --- |
| `TimeFormat` | 0 = 12-hour, 1 = 24-hour |
| `ShowSeconds`, `AlwaysOnTop`, `ClockVisible` | Boolean flags |
| `TextColor` | RGB value |
| `Transparency` | Background opacity, 0–255 |
| `TextAlpha` | Text opacity, 0–255 |
| `FontName` | Font family name |
| `FontSize` | 0 = Small, 1 = Medium, 2 = Large |
| `WindowX`, `WindowY`, `WindowWidth`, `WindowHeight` | Position and size |

The startup entry, when enabled, is written to `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`.

Deleting the `Software\DesktopClock` key resets everything to defaults on the next launch.

## Building from Source

The build targets Windows x64 but runs on Linux (or WSL) using the MinGW-w64 cross-compiler.

```bash
sudo apt update
sudo apt install mingw-w64
./build.sh
```

`build.sh` compiles the resources with `windres`, links with `x86_64-w64-mingw32-g++`, and writes `DesktopClock.exe` to the project root. Copy it to a Windows machine to run it.

The C++ runtime is linked statically (`-static-libgcc -static-libstdc++`), so the executable has no DLL dependencies beyond the system libraries.

## Project Structure

```
desktop-clock/
├── src/
│   ├── main.cpp              # Entry point, dialog procedures
│   ├── clock_window.h/cpp    # Clock window and input handling
│   ├── settings.h/cpp        # Registry-backed settings
│   ├── renderer.h/cpp        # GDI+ text rendering
│   └── tray_icon.h/cpp       # System tray integration
├── resources/
│   ├── clock.rc              # Dialogs, menus, resource definitions
│   ├── resource.h            # Resource IDs
│   ├── app_strings.h         # Application strings and version
│   ├── clock_icon.ico        # Application icon
│   └── tray_icon.ico         # Tray icon
├── build.sh                  # Cross-compilation script
└── LICENSE
```

### How it works

A single-threaded, pure Win32 application. The clock is a `WS_EX_LAYERED` window whose transparency comes from `SetLayeredWindowAttributes()`, with digits drawn anti-aliased through GDI+. A `SetTimer()` tick repaints once a second when seconds are shown and once a minute otherwise. Tray integration goes through `Shell_NotifyIcon()`.

## Troubleshooting

**Build: `MinGW-w64 not found`** — install it with `sudo apt install mingw-w64`, then confirm with `x86_64-w64-mingw32-g++ --version`. The same package provides `windres`.

**Build fails on `build.sh`** — check that it is executable: `chmod +x build.sh`.

**The clock doesn't appear** — it may be hidden; right-click the tray icon and choose **Show Clock**. If it opened off-screen (for example after a monitor change), delete `HKEY_CURRENT_USER\Software\DesktopClock` and restart to re-center it.

**No tray icon** — check your Windows notification-area settings, or restart Explorer from Task Manager.

**Settings don't persist** — verify your user account can write to `HKEY_CURRENT_USER`.

## Known Limitations

- One clock instance at a time
- Position is remembered for the primary monitor only
- Fonts limited to the seven listed above
- No date display, alarms, or keyboard shortcuts

## Contributing

Issues and pull requests are welcome. Please test changes on Windows 10 or 11, and follow the existing code style.

## Changelog

### v1.1.0

**Fixes**
- **Apply** now updates the clock immediately instead of waiting for the next timer tick
- The color preview swatch updates correctly when a new color is chosen
- The preview section reflects all setting changes

**Enhancements**
- Separate background and text opacity controls (`TextAlpha`)
- Font selection across seven families (`FontName`)
- Clearer settings dialog layout and labels

Existing v1.0 settings are read unchanged.

### v1.0

Initial release: floating transparent window, 12/24-hour format, optional seconds, custom color and transparency, resizable with auto-scaling font, tray integration, persistent settings, always-on-top, and start-with-Windows.

## License

MIT — see [LICENSE](LICENSE).

Built with the Win32 API, GDI+, and MinGW-w64.
