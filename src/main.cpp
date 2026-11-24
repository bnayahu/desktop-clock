#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <stdio.h>
#include "clock_window.h"
#include "settings.h"
#include "tray_icon.h"
#include "renderer.h"
#include "world_clock.h"
#include "../resources/resource.h"
#include "../resources/app_strings.h"

// Global instances
SettingsManager* g_settingsMgr = nullptr;
ClockWindow* g_clockWindow = nullptr;
TrayIcon* g_trayIcon = nullptr;

// Forward declarations
INT_PTR CALLBACK SettingsDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void UpdatePreview(HWND hwndDlg, const ClockSettings* settings);
void UpdateAlphaLabel(HWND hwndDlg, int alpha);
void UpdateTextAlphaLabel(HWND hwndDlg, int alpha);
void UpdateColorPreviewBox(HWND hwndDlg, COLORREF color);
void InitWorldZoneControls(HWND hwndDlg, ClockSettings* settings);
void UpdateSplitLabel(HWND hwndDlg, int percent);

// World clock dialog controls, indexed by zone.
static const int kTzCombos[WORLD_ZONE_COUNT] = { IDC_COMBO_TZ1, IDC_COMBO_TZ2 };
static const int kLabelEdits[WORLD_ZONE_COUNT] = { IDC_EDIT_LABEL1, IDC_EDIT_LABEL2 };

// System timezone list, loaded once on first use of the settings dialog.
static const int MAX_TIME_ZONES = 256;
static TimeZoneEntry g_timeZones[MAX_TIME_ZONES];
static int g_timeZoneCount = -1;

// Main window procedure for hidden window
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TRAYICON:
            if (g_trayIcon) {
                g_trayIcon->HandleTrayMessage(wParam, lParam);
            }
            return 0;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_SHOW_HIDE:
                    if (g_trayIcon) g_trayIcon->OnShowHide();
                    break;
                case IDM_SETTINGS:
                    if (g_trayIcon) g_trayIcon->OnSettings();
                    break;
                case IDM_TOPMOST:
                    if (g_trayIcon) g_trayIcon->OnToggleTopmost();
                    break;
                case IDM_STARTUP:
                    if (g_trayIcon) g_trayIcon->OnToggleStartup();
                    break;
                case IDM_ABOUT:
                    if (g_trayIcon) g_trayIcon->OnAbout();
                    break;
                case IDM_EXIT:
                    if (g_trayIcon) g_trayIcon->OnExit();
                    break;
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);
    
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    // Create settings manager
    g_settingsMgr = new SettingsManager();
    g_settingsMgr->LoadSettings();
    
    // Register hidden window class for message handling
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DesktopClockMainWindow";
    
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Failed to register main window class!", 
                  APP_NAME, MB_ICONERROR | MB_OK);
        return 1;
    }
    
    // Create hidden window for message processing
    HWND hwndMain = CreateWindowEx(
        0,
        L"DesktopClockMainWindow",
        APP_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    
    if (hwndMain == NULL) {
        MessageBox(NULL, L"Failed to create main window!", 
                  APP_NAME, MB_ICONERROR | MB_OK);
        return 1;
    }
    
    // Create clock window
    g_clockWindow = new ClockWindow(hInstance, g_settingsMgr);
    if (!g_clockWindow->Create()) {
        MessageBox(NULL, L"Failed to create clock window!", 
                  APP_NAME, MB_ICONERROR | MB_OK);
        return 1;
    }
    
    // Create tray icon
    g_trayIcon = new TrayIcon(hwndMain, g_clockWindow, g_settingsMgr);
    if (!g_trayIcon->Create()) {
        MessageBox(NULL, L"Failed to create tray icon!",
                  APP_NAME, MB_ICONERROR | MB_OK);
        return 1;
    }
    
    // Show clock if it was visible last time
    if (g_settingsMgr->GetSettings().clockVisible) {
        g_clockWindow->Show();
    }
    
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    delete g_trayIcon;
    delete g_clockWindow;
    delete g_settingsMgr;
    
    Gdiplus::GdiplusShutdown(gdiplusToken);
    
    return (int)msg.wParam;
}

// Settings dialog procedure
INT_PTR CALLBACK SettingsDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static SettingsManager* settingsMgr = nullptr;
    static ClockSettings tempSettings;
    
    switch (msg) {
        case WM_INITDIALOG: {
            // Store settings manager pointer
            settingsMgr = (SettingsManager*)lParam;
            tempSettings = settingsMgr->GetSettings();
            
            // Initialize time format radio buttons
            CheckRadioButton(hwndDlg, IDC_RADIO_12H, IDC_RADIO_24H,
                           tempSettings.timeFormat == TIME_12H ? 
                           IDC_RADIO_12H : IDC_RADIO_24H);
            
            // Initialize checkboxes
            CheckDlgButton(hwndDlg, IDC_CHECK_SECONDS,
                          tempSettings.showSeconds ? BST_CHECKED : BST_UNCHECKED);
            
            CheckDlgButton(hwndDlg, IDC_CHECK_TOPMOST,
                          tempSettings.alwaysOnTop ? BST_CHECKED : BST_UNCHECKED);
            
            CheckDlgButton(hwndDlg, IDC_CHECK_STARTUP,
                          settingsMgr->IsStartupEnabled() ? 
                          BST_CHECKED : BST_UNCHECKED);
            
            // Initialize background transparency slider
            SendDlgItemMessage(hwndDlg, IDC_SLIDER_ALPHA, TBM_SETRANGE,
                             TRUE, MAKELONG(0, 255));
            SendDlgItemMessage(hwndDlg, IDC_SLIDER_ALPHA, TBM_SETPOS,
                             TRUE, tempSettings.transparency);
            UpdateAlphaLabel(hwndDlg, tempSettings.transparency);
            
            // Initialize text alpha slider
            SendDlgItemMessage(hwndDlg, IDC_SLIDER_TEXT_ALPHA, TBM_SETRANGE,
                             TRUE, MAKELONG(0, 255));
            SendDlgItemMessage(hwndDlg, IDC_SLIDER_TEXT_ALPHA, TBM_SETPOS,
                             TRUE, tempSettings.textAlpha);
            UpdateTextAlphaLabel(hwndDlg, tempSettings.textAlpha);
            
            // Initialize font name combo
            const wchar_t* fonts[] = {
                L"Segoe UI", L"Arial", L"Calibri", L"Consolas",
                L"Courier New", L"Times New Roman", L"Verdana"
            };
            int selectedFont = 0;
            for (int i = 0; i < 7; i++) {
                SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTNAME, CB_ADDSTRING,
                                 0, (LPARAM)fonts[i]);
                if (wcscmp(tempSettings.fontName, fonts[i]) == 0) {
                    selectedFont = i;
                }
            }
            SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTNAME, CB_SETCURSEL,
                             selectedFont, 0);
            
            // Initialize font size combo
            SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTSIZE, CB_ADDSTRING,
                             0, (LPARAM)L"Small");
            SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTSIZE, CB_ADDSTRING,
                             0, (LPARAM)L"Medium");
            SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTSIZE, CB_ADDSTRING,
                             0, (LPARAM)L"Large");
            SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTSIZE, CB_SETCURSEL,
                             tempSettings.fontSize, 0);
            
            // Initialize world clock zones and labels
            InitWorldZoneControls(hwndDlg, &tempSettings);
            
            // Update preview and color box
            UpdateColorPreviewBox(hwndDlg, tempSettings.textColor);
            UpdatePreview(hwndDlg, &tempSettings);
            
            return TRUE;
        }
        
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_BUTTON_COLOR: {
                    CHOOSECOLOR cc;
                    static COLORREF customColors[16];
                    
                    ZeroMemory(&cc, sizeof(CHOOSECOLOR));
                    cc.lStructSize = sizeof(CHOOSECOLOR);
                    cc.hwndOwner = hwndDlg;
                    cc.lpCustColors = customColors;
                    cc.rgbResult = tempSettings.textColor;
                    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                    
                    if (ChooseColor(&cc)) {
                        tempSettings.textColor = cc.rgbResult;
                        UpdateColorPreviewBox(hwndDlg, tempSettings.textColor);
                        UpdatePreview(hwndDlg, &tempSettings);
                    }
                    break;
                }
                
                case IDC_RADIO_12H:
                case IDC_RADIO_24H:
                    tempSettings.timeFormat = (IsDlgButtonChecked(hwndDlg, IDC_RADIO_12H) == BST_CHECKED) 
                                             ? TIME_12H : TIME_24H;
                    UpdatePreview(hwndDlg, &tempSettings);
                    break;
                
                case IDC_CHECK_SECONDS:
                    tempSettings.showSeconds = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SECONDS) == BST_CHECKED);
                    UpdatePreview(hwndDlg, &tempSettings);
                    break;
                
                case IDC_COMBO_FONTNAME:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        int sel = SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTNAME, CB_GETCURSEL, 0, 0);
                        if (sel != CB_ERR) {
                            wchar_t fontName[32];
                            SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTNAME, CB_GETLBTEXT, sel, (LPARAM)fontName);
                            wcscpy_s(tempSettings.fontName, 32, fontName);
                            UpdatePreview(hwndDlg, &tempSettings);
                        }
                    }
                    break;
                
                case IDC_COMBO_FONTSIZE:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        int sel = SendDlgItemMessage(hwndDlg, IDC_COMBO_FONTSIZE, CB_GETCURSEL, 0, 0);
                        if (sel != CB_ERR) {
                            tempSettings.fontSize = (FontSize)sel;
                            UpdatePreview(hwndDlg, &tempSettings);
                        }
                    }
                    break;
                
                case IDC_COMBO_TZ1:
                case IDC_COMBO_TZ2:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        int zone = (LOWORD(wParam) == IDC_COMBO_TZ1) ? 0 : 1;
                        int sel = SendDlgItemMessage(hwndDlg, kTzCombos[zone], CB_GETCURSEL, 0, 0);
                        LRESULT index = (sel == CB_ERR) ? -1 :
                            SendDlgItemMessage(hwndDlg, kTzCombos[zone], CB_GETITEMDATA, sel, 0);
                        if (index >= 0 && index < g_timeZoneCount) {
                            wcsncpy(tempSettings.worldTz[zone], g_timeZones[index].keyName,
                                    WORLD_TZ_KEY_LEN - 1);
                            tempSettings.worldTz[zone][WORLD_TZ_KEY_LEN - 1] = L'\0';
                        } else {
                            tempSettings.worldTz[zone][0] = L'\0';
                        }
                        UpdatePreview(hwndDlg, &tempSettings);
                    }
                    break;
                
                case IDC_EDIT_LABEL1:
                case IDC_EDIT_LABEL2:
                    if (HIWORD(wParam) == EN_CHANGE) {
                        int zone = (LOWORD(wParam) == IDC_EDIT_LABEL1) ? 0 : 1;
                        GetDlgItemText(hwndDlg, kLabelEdits[zone],
                                       tempSettings.worldLabel[zone], WORLD_LABEL_MAX + 1);
                        UpdatePreview(hwndDlg, &tempSettings);
                    }
                    break;
                
                case IDOK:
                case IDAPPLY: {
                    // Get checkbox states
                    tempSettings.alwaysOnTop = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_TOPMOST) == BST_CHECKED);
                    bool startupEnabled = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_STARTUP) == BST_CHECKED);
                    
                    // Apply settings
                    settingsMgr->GetSettings() = tempSettings;
                    settingsMgr->SaveSettings();
                    settingsMgr->SetStartupEnabled(startupEnabled);
                    
                    // Apply settings to clock window immediately (fixes Apply button bug)
                    if (g_clockWindow) {
                        g_clockWindow->ApplySettings();
                    }
                    
                    if (LOWORD(wParam) == IDOK) {
                        EndDialog(hwndDlg, IDOK);
                    }
                    break;
                }
                
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    break;
            }
            break;
        
        case WM_HSCROLL:
            // Handle background transparency slider
            if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER_ALPHA)) {
                int pos = SendDlgItemMessage(hwndDlg, IDC_SLIDER_ALPHA,
                                            TBM_GETPOS, 0, 0);
                tempSettings.transparency = (BYTE)pos;
                UpdateAlphaLabel(hwndDlg, pos);
                UpdatePreview(hwndDlg, &tempSettings);
            }
            // Handle text alpha slider
            else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER_TEXT_ALPHA)) {
                int pos = SendDlgItemMessage(hwndDlg, IDC_SLIDER_TEXT_ALPHA,
                                            TBM_GETPOS, 0, 0);
                tempSettings.textAlpha = (BYTE)pos;
                UpdateTextAlphaLabel(hwndDlg, pos);
                UpdatePreview(hwndDlg, &tempSettings);
            }
            // Handle main-time / world-row split slider
            else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER_SPLIT)) {
                int pos = SendDlgItemMessage(hwndDlg, IDC_SLIDER_SPLIT,
                                            TBM_GETPOS, 0, 0);
                tempSettings.worldSplit = pos;
                UpdateSplitLabel(hwndDlg, pos);
            }
            break;
    }
    
    return FALSE;
}

// About dialog procedure
INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            // Built from the version macros so this can never disagree with
            // the version reported in the executable's file properties.
            wchar_t versionText[64];
            wsprintf(versionText, L"%s v%d.%d",
                    APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR);
            SetDlgItemText(hwndDlg, IDC_STATIC_ABOUT_VERSION, versionText);
            return TRUE;
        }
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    
    return FALSE;
}

// Helper function to update preview
void UpdatePreview(HWND hwndDlg, const ClockSettings* settings) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    wchar_t timeStr[32];
    if (settings->timeFormat == TIME_12H) {
        int hour = st.wHour % 12;
        if (hour == 0) hour = 12;
        
        if (settings->showSeconds) {
            wsprintf(timeStr, L"%d:%02d:%02d %s",
                    hour, st.wMinute, st.wSecond,
                    st.wHour >= 12 ? L"PM" : L"AM");
        } else {
            wsprintf(timeStr, L"%d:%02d %s",
                    hour, st.wMinute,
                    st.wHour >= 12 ? L"PM" : L"AM");
        }
    } else {
        if (settings->showSeconds) {
            wsprintf(timeStr, L"%02d:%02d:%02d",
                    st.wHour, st.wMinute, st.wSecond);
        } else {
            wsprintf(timeStr, L"%02d:%02d",
                    st.wHour, st.wMinute);
        }
    }
    
    // Second line: the world clock row, if any zone is configured
    wchar_t worldRow[160];
    if (BuildWorldRow(*settings, st, worldRow, 160)) {
        wchar_t previewStr[200];
        swprintf(previewStr, 200, L"%ls\r\n%ls", timeStr, worldRow);
        SetDlgItemText(hwndDlg, IDC_STATIC_PREVIEW, previewStr);
    } else {
        SetDlgItemText(hwndDlg, IDC_STATIC_PREVIEW, timeStr);
    }
}

// Fills the world clock zone combos and label edits from settings.
// A stored zone that is no longer a known timezone shows as "(None)" and is
// cleared, so saving the dialog doesn't write the stale key back.
void InitWorldZoneControls(HWND hwndDlg, ClockSettings* settings) {
    if (g_timeZoneCount < 0) {
        g_timeZoneCount = EnumerateTimeZones(g_timeZones, MAX_TIME_ZONES);
    }
    
    for (int zone = 0; zone < WORLD_ZONE_COUNT; zone++) {
        HWND combo = GetDlgItem(hwndDlg, kTzCombos[zone]);
        int item = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)L"(None)");
        SendMessage(combo, CB_SETITEMDATA, item, (LPARAM)-1);
        int selected = item;
        
        for (int i = 0; i < g_timeZoneCount; i++) {
            item = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)g_timeZones[i].displayName);
            SendMessage(combo, CB_SETITEMDATA, item, (LPARAM)i);
            if (wcscmp(settings->worldTz[zone], g_timeZones[i].keyName) == 0) {
                selected = item;
            }
        }
        SendMessage(combo, CB_SETCURSEL, selected, 0);
        if (selected == 0) {
            settings->worldTz[zone][0] = L'\0';
        }
        
        SendDlgItemMessage(hwndDlg, kLabelEdits[zone], EM_LIMITTEXT, WORLD_LABEL_MAX, 0);
        SetDlgItemText(hwndDlg, kLabelEdits[zone], settings->worldLabel[zone]);
    }
    
    SendDlgItemMessage(hwndDlg, IDC_SLIDER_SPLIT, TBM_SETRANGE,
                       TRUE, MAKELONG(WORLD_SPLIT_MIN, WORLD_SPLIT_MAX));
    SendDlgItemMessage(hwndDlg, IDC_SLIDER_SPLIT, TBM_SETTICFREQ, 5, 0);
    SendDlgItemMessage(hwndDlg, IDC_SLIDER_SPLIT, TBM_SETPOS, TRUE, settings->worldSplit);
    UpdateSplitLabel(hwndDlg, settings->worldSplit);
}

// Helper function to update the main-time size (split) label
void UpdateSplitLabel(HWND hwndDlg, int percent) {
    wchar_t text[16];
    wsprintf(text, L"%d%%", percent);
    SetDlgItemText(hwndDlg, IDC_STATIC_SPLIT_VALUE, text);
}

// Helper function to update background alpha label
void UpdateAlphaLabel(HWND hwndDlg, int alpha) {
    wchar_t alphaStr[16];
    wsprintf(alphaStr, L"%d", alpha);
    SetDlgItemText(hwndDlg, IDC_STATIC_ALPHA_VALUE, alphaStr);
}

// Helper function to update text alpha label
void UpdateTextAlphaLabel(HWND hwndDlg, int alpha) {
    wchar_t alphaStr[16];
    wsprintf(alphaStr, L"%d", alpha);
    SetDlgItemText(hwndDlg, IDC_STATIC_TEXT_ALPHA_VALUE, alphaStr);
}

// Helper function to update color preview box
void UpdateColorPreviewBox(HWND hwndDlg, COLORREF color) {
    HWND hColorBox = GetDlgItem(hwndDlg, IDC_STATIC_COLOR_BOX);
    if (hColorBox) {
        HDC hdc = GetDC(hColorBox);
        if (hdc) {
            RECT rect;
            GetClientRect(hColorBox, &rect);
            HBRUSH hBrush = CreateSolidBrush(color);
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);
            ReleaseDC(hColorBox, hdc);
        }
    }
}

// Made with Bob
