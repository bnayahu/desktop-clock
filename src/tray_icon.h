#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <windows.h>
#include <shellapi.h>

class ClockWindow;
class SettingsManager;

class TrayIcon {
public:
    TrayIcon(HWND hwnd, ClockWindow* clockWindow, SettingsManager* settingsMgr);
    ~TrayIcon();
    
    bool Create();
    void Remove();
    void ShowContextMenu(POINT pt);
    void HandleTrayMessage(WPARAM wParam, LPARAM lParam);
    
    // Public methods for menu actions
    void OnShowHide();
    void OnSettings();
    void OnToggleTopmost();
    void OnToggleStartup();
    void OnAbout();
    void OnExit();
    
private:
    void UpdateMenuChecks(HMENU hMenu);
    
    HWND m_hwnd;
    ClockWindow* m_clockWindow;
    SettingsManager* m_settingsMgr;
    NOTIFYICONDATA m_nid;
};

#endif // TRAY_ICON_H

// Made with Bob
