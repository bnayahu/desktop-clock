#include "tray_icon.h"
#include "clock_window.h"
#include "settings.h"
#include "../resources/resource.h"
#include "../resources/app_strings.h"

// Forward declarations for dialog procedures
INT_PTR CALLBACK SettingsDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

TrayIcon::TrayIcon(HWND hwnd, ClockWindow* clockWindow, SettingsManager* settingsMgr)
    : m_hwnd(hwnd)
    , m_clockWindow(clockWindow)
    , m_settingsMgr(settingsMgr) {
    ZeroMemory(&m_nid, sizeof(NOTIFYICONDATA));
}

TrayIcon::~TrayIcon() {
    Remove();
}

bool TrayIcon::Create() {
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRAY_ICON));
    
    wcsncpy(m_nid.szTip, APP_TRAY_TOOLTIP, sizeof(m_nid.szTip) / sizeof(wchar_t) - 1);
    m_nid.szTip[sizeof(m_nid.szTip) / sizeof(wchar_t) - 1] = L'\0';
    
    return Shell_NotifyIcon(NIM_ADD, &m_nid) != FALSE;
}

void TrayIcon::Remove() {
    Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void TrayIcon::HandleTrayMessage(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(lParam)) {
        case WM_LBUTTONDBLCLK:
            OnShowHide();
            break;
            
        case WM_RBUTTONUP:
            POINT pt;
            GetCursorPos(&pt);
            ShowContextMenu(pt);
            break;
    }
}

void TrayIcon::ShowContextMenu(POINT pt) {
    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        // Add menu items
        AppendMenu(hMenu, MF_STRING, IDM_SHOW_HIDE, 
                  m_clockWindow->IsVisible() ? L"Hide Clock" : L"Show Clock");
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, MF_STRING, IDM_SETTINGS, L"Settings...");
        AppendMenu(hMenu, MF_STRING, IDM_TOPMOST, L"Always on Top");
        AppendMenu(hMenu, MF_STRING, IDM_STARTUP, L"Start with Windows");
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, MF_STRING, IDM_ABOUT, L"About");
        AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");
        
        UpdateMenuChecks(hMenu);
        
        // Required for proper menu behavior
        SetForegroundWindow(m_hwnd);
        
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, 
                      pt.x, pt.y, 0, m_hwnd, NULL);
        
        DestroyMenu(hMenu);
    }
}

void TrayIcon::UpdateMenuChecks(HMENU hMenu) {
    const ClockSettings& settings = m_settingsMgr->GetSettings();
    
    CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | 
                 (settings.alwaysOnTop ? MF_CHECKED : MF_UNCHECKED));
    
    CheckMenuItem(hMenu, IDM_STARTUP, MF_BYCOMMAND | 
                 (m_settingsMgr->IsStartupEnabled() ? MF_CHECKED : MF_UNCHECKED));
}

void TrayIcon::OnShowHide() {
    if (m_clockWindow->IsVisible()) {
        m_clockWindow->Hide();
    } else {
        m_clockWindow->Show();
    }
}

void TrayIcon::OnSettings() {
    DialogBoxParam(GetModuleHandle(NULL), 
                  MAKEINTRESOURCE(IDD_SETTINGS),
                  m_hwnd,
                  SettingsDialogProc,
                  (LPARAM)m_settingsMgr);
    
    // Apply any changes
    m_clockWindow->ApplySettings();
}

void TrayIcon::OnToggleTopmost() {
    ClockSettings& settings = m_settingsMgr->GetSettings();
    settings.alwaysOnTop = !settings.alwaysOnTop;
    m_settingsMgr->SaveSettings();
    m_clockWindow->ApplySettings();
}

void TrayIcon::OnToggleStartup() {
    bool currentState = m_settingsMgr->IsStartupEnabled();
    m_settingsMgr->SetStartupEnabled(!currentState);
}

void TrayIcon::OnAbout() {
    DialogBox(GetModuleHandle(NULL),
             MAKEINTRESOURCE(IDD_ABOUT),
             m_hwnd,
             AboutDialogProc);
}

void TrayIcon::OnExit() {
    PostQuitMessage(0);
}

// Made with Bob
