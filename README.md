# Desktop Clock

A lightweight, customizable floating digital clock widget for Windows 10/11. Built with Win32 API and cross-compiled in WSL using MinGW-w64.

![Desktop Clock](https://img.shields.io/badge/platform-Windows%2010%2F11-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Features

- **Floating Window**: Transparent, always-accessible clock that floats above other windows
- **Customizable Display**:
  - 12-hour or 24-hour time format
  - Optional seconds display
  - Adjustable transparency (0-255)
  - Custom text color (RGB picker or theme-based)
  - Three font sizes (Small, Medium, Large)
- **Resizable**: Drag window edges to resize; font auto-scales
- **Draggable**: Click and drag anywhere to reposition
- **System Tray Integration**: Minimize to tray with quick access menu
- **Persistent Settings**: All preferences saved to Windows Registry
- **Optional Features**:
  - Always on top mode
  - Start with Windows
  - Remember position and size between sessions

## Screenshots

```
┌─────────────────────────┐
│                         │
│      14:35:42           │  ← Centered, scalable text
│                         │     Customizable color
│                         │     Adjustable transparency
└─────────────────────────┘
```

## Requirements

### Runtime
- Windows 10 or Windows 11
- No additional runtime dependencies required

### Building
- Ubuntu/Debian Linux (or WSL)
- MinGW-w64 cross-compiler toolchain
- GCC/G++ compiler

## Installation

### Pre-built Binary
1. Download `DesktopClock.exe` from releases
2. Place it anywhere on your system
3. Double-click to run
4. The clock appears and a tray icon is added

### Building from Source

#### 1. Install MinGW-w64

On Ubuntu/Debian/WSL:
```bash
sudo apt update
sudo apt install mingw-w64
```

#### 2. Clone and Build

```bash
cd /path/to/desktop-clock
./build.sh
```

The script will:
- Check for MinGW-w64 installation
- Compile resources with `windres`
- Compile and link with `x86_64-w64-mingw32-g++`
- Create `DesktopClock.exe` in the project root

#### 3. Transfer to Windows

Copy `DesktopClock.exe` to your Windows machine and run it.

## Usage

### First Run

1. Launch `DesktopClock.exe`
2. The clock window appears at the center of your screen
3. A tray icon appears in the system tray

### Basic Operations

- **Move Clock**: Click and drag anywhere on the clock window
- **Resize Clock**: Drag the window edges or corners (font auto-scales)
- **Show/Hide**: Double-click the tray icon
- **Settings**: Right-click tray icon → Settings
- **Exit**: Right-click tray icon → Exit

### Tray Icon Menu

Right-click the tray icon to access:
- **Show/Hide Clock**: Toggle clock visibility
- **Settings...**: Open configuration dialog
- **Always on Top**: Toggle topmost window mode
- **Start with Windows**: Toggle startup behavior
- **About**: Display application information
- **Exit**: Close the application

### Settings Dialog

Configure all aspects of the clock:

**Time Format**
- 12-hour (with AM/PM)
- 24-hour

**Display Options**
- Show seconds (updates every second vs. every minute)

**Window Options**
- Always on top (clock stays above other windows)

**Appearance**
- Text Color: Click "Choose Color..." to select custom color
- Transparency: Slider from 0 (invisible) to 255 (opaque)
- Font Size: Small, Medium, or Large (scales with window size)

**Startup**
- Start Desktop Clock when Windows starts

**Preview**
- Live preview of current settings

### Keyboard Shortcuts

Currently, all operations are mouse-based. Future versions may add keyboard shortcuts.

## Configuration

Settings are stored in the Windows Registry:
- **Settings Key**: `HKEY_CURRENT_USER\Software\DesktopClock`
- **Startup Key**: `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`

Stored values include:
- Time format (12h/24h)
- Show seconds (yes/no)
- Always on top (yes/no)
- Text color (RGB value)
- Transparency level (0-255)
- Font size (Small/Medium/Large)
- Window position (X, Y)
- Window size (Width, Height)
- Clock visibility state

## Project Structure

```
desktop-clock/
├── src/
│   ├── main.cpp              # Entry point and dialog procedures
│   ├── clock_window.h/cpp    # Clock window implementation
│   ├── settings.h/cpp        # Settings management
│   ├── renderer.h/cpp        # GDI+ rendering
│   └── tray_icon.h/cpp       # System tray integration
├── resources/
│   ├── clock.rc              # Resource definition
│   ├── resource.h            # Resource IDs
│   ├── app_strings.h         # Application strings
│   ├── clock_icon.ico        # Application icon
│   └── tray_icon.ico         # System tray icon
├── build.sh                  # Build script
├── .gitignore               # Git ignore file
├── README.md                # This file
├── DESIGN.md                # Design document
├── IMPLEMENTATION_PLAN.md   # Implementation guide
├── ARCHITECTURE.md          # Architecture documentation
└── LICENSE                  # License file
```

## Technical Details

### Architecture
- Pure Win32 API application
- GDI+ for anti-aliased text rendering
- Layered window with per-pixel alpha transparency
- Single-threaded design for simplicity
- Timer-based updates (1 second or 1 minute intervals)
- Registry-based settings persistence

### Key Windows APIs Used
- `CreateWindowEx()` with `WS_EX_LAYERED`: Transparent window
- `SetLayeredWindowAttributes()`: Transparency control
- `Shell_NotifyIcon()`: System tray integration
- GDI+ Graphics API: Text rendering
- Registry API: Settings storage
- `SetTimer()`: Periodic updates

### Cross-Compilation
- Built on Linux using MinGW-w64
- Targets Windows x64 platform
- Static linking for portability (no DLL dependencies)
- GCC/G++ compiler with Windows headers

## Performance

- **CPU Usage**: <1% when idle
- **Memory**: ~5-10 MB
- **Startup Time**: Instant
- **Update Frequency**: 
  - With seconds: Every 1 second
  - Without seconds: Every 1 minute

## Troubleshooting

### Build Issues

**MinGW-w64 not found**
```bash
sudo apt install mingw-w64
x86_64-w64-mingw32-g++ --version  # Verify installation
```

**windres command not found**
- MinGW-w64 package includes windres
- Check: `which x86_64-w64-mingw32-windres`

**Compilation errors**
- Ensure all source files are present
- Check file permissions on build.sh: `chmod +x build.sh`
- Review error output for specific issues

### Runtime Issues (on Windows)

**Application won't start**
- Ensure you're running on Windows 10 or later
- Try running as administrator (though it shouldn't be required)
- Check Windows Event Viewer for errors

**Clock doesn't appear**
- Check if it's hidden: Right-click tray icon → Show Clock
- Try resetting position: Delete registry key and restart

**Tray icon doesn't appear**
- Check Windows notification settings
- Restart Windows Explorer: `Ctrl+Shift+Esc` → Windows Explorer → Restart

**Settings don't persist**
- Check registry permissions
- Try running as administrator once to create registry keys

**Clock appears off-screen**
- Delete registry key: `HKEY_CURRENT_USER\Software\DesktopClock`
- Restart application (will center on screen)

## Known Limitations

- Single clock instance only
- No multi-monitor position memory (uses primary monitor)
- No custom font selection (uses Segoe UI)
- No date display
- No alarm/reminder functionality

## Future Enhancements

Potential features for future versions:
- Multiple clock instances with different time zones
- Date display option
- Custom font selection
- Themes/skins support
- Keyboard shortcuts
- Animation effects
- Sound on hour change
- Calendar integration
- Multi-monitor support with position per monitor

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on Windows 10/11
5. Submit a pull request

### Code Style

- Follow existing code style
- Use meaningful variable names
- Comment complex logic
- Keep functions focused and small

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Credits

Desktop Clock - A customizable floating clock widget for Windows.

Built with:
- Win32 API
- GDI+ Graphics Library
- MinGW-w64 cross-compiler

Inspired by the need for a simple, customizable desktop clock that doesn't require installation or external dependencies.

## Support

For issues, questions, or suggestions:
- Open an issue on GitHub
- Check existing issues for solutions
- Review the troubleshooting section above

## Changelog

### Version 1.0 (Initial Release)
- Floating, transparent clock window
- 12/24-hour format support
- Optional seconds display
- Customizable color and transparency
- Resizable window with auto-scaling font
- System tray integration
- Settings persistence
- Always on top option
- Start with Windows option
- Position and size memory

---

**Enjoy your new Desktop Clock!** ⏰