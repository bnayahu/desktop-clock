#ifndef APP_STRINGS_H
#define APP_STRINGS_H

#define APP_NAME                L"Desktop Clock"

// Version: the single source of truth. Update these two numbers when
// releasing; the About dialog and the VS_VERSION_INFO resource both derive
// their text from them, so they cannot drift apart.
#define APP_VERSION_MAJOR       1
#define APP_VERSION_MINOR       2

// Turns the numbers above into resource strings.
#define APP_STRINGIFY_(x)       #x
#define APP_STRINGIFY(x)        APP_STRINGIFY_(x)

#define APP_VERSION_STR         APP_STRINGIFY(APP_VERSION_MAJOR) "." APP_STRINGIFY(APP_VERSION_MINOR)
#define APP_VERSION_STR_FULL    APP_VERSION_STR ".0.0"

#define APP_COPYRIGHT           "Copyright 2026 Jonathan Bnayahu. Licensed under Apache-2.0."

#define APP_WINDOW_CLASS        L"DesktopClockWindowClass"
#define APP_REGISTRY_KEY        L"Software\\DesktopClock"
#define APP_REGISTRY_VALUE      L"DesktopClock"
#define APP_STARTUP_KEY         L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

#define APP_TRAY_TOOLTIP        L"Desktop Clock - Right-click for options"

#endif // APP_STRINGS_H
