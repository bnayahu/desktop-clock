#include "settings.h"
#include "../resources/app_strings.h"
#include <stdio.h>

SettingsManager::SettingsManager() {
    InitializeDefaults();
}

SettingsManager::~SettingsManager() {
}

void SettingsManager::InitializeDefaults() {
    m_settings.timeFormat = TIME_24H;
    m_settings.showSeconds = false;
    m_settings.alwaysOnTop = false;
    m_settings.textColor = GetSysColor(COLOR_WINDOWTEXT);
    m_settings.transparency = 255;
    m_settings.fontSize = FONT_SIZE_MEDIUM;
    m_settings.windowX = CW_USEDEFAULT;
    m_settings.windowY = CW_USEDEFAULT;
    m_settings.windowWidth = 200;
    m_settings.windowHeight = 80;
    m_settings.startWithWindows = false;
    m_settings.clockVisible = true;
}

void SettingsManager::LoadSettings() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, APP_REGISTRY_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD value;
        DWORD size = sizeof(DWORD);
        DWORD type;
        
        if (RegQueryValueEx(hKey, L"TimeFormat", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD && (value == TIME_12H || value == TIME_24H)) {
                m_settings.timeFormat = (TimeFormat)value;
            }
        }
        
        if (RegQueryValueEx(hKey, L"ShowSeconds", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD) {
                m_settings.showSeconds = (value != 0);
            }
        }
        
        if (RegQueryValueEx(hKey, L"AlwaysOnTop", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD) {
                m_settings.alwaysOnTop = (value != 0);
            }
        }
        
        if (RegQueryValueEx(hKey, L"TextColor", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD) {
                m_settings.textColor = (COLORREF)value;
            }
        }
        
        if (RegQueryValueEx(hKey, L"Transparency", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD && value <= 255) {
                m_settings.transparency = (BYTE)value;
            }
        }
        
        if (RegQueryValueEx(hKey, L"FontSize", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD && value <= FONT_SIZE_LARGE) {
                m_settings.fontSize = (FontSize)value;
            }
        }
        
        if (RegQueryValueEx(hKey, L"WindowX", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD) {
                m_settings.windowX = (int)value;
            }
        }
        
        if (RegQueryValueEx(hKey, L"WindowY", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD) {
                m_settings.windowY = (int)value;
            }
        }
        
        if (RegQueryValueEx(hKey, L"WindowWidth", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD && value >= 100 && value <= 2000) {
                m_settings.windowWidth = (int)value;
            }
        }
        
        if (RegQueryValueEx(hKey, L"WindowHeight", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD && value >= 50 && value <= 1000) {
                m_settings.windowHeight = (int)value;
            }
        }
        
        if (RegQueryValueEx(hKey, L"ClockVisible", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            if (type == REG_DWORD) {
                m_settings.clockVisible = (value != 0);
            }
        }
        
        RegCloseKey(hKey);
    }
    
    // Validate window position is on screen
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    if (m_settings.windowX != CW_USEDEFAULT) {
        if (m_settings.windowX < -m_settings.windowWidth + 50 || 
            m_settings.windowX > screenWidth - 50) {
            m_settings.windowX = CW_USEDEFAULT;
            m_settings.windowY = CW_USEDEFAULT;
        }
    }
    
    if (m_settings.windowY != CW_USEDEFAULT) {
        if (m_settings.windowY < 0 || 
            m_settings.windowY > screenHeight - 50) {
            m_settings.windowX = CW_USEDEFAULT;
            m_settings.windowY = CW_USEDEFAULT;
        }
    }
}

void SettingsManager::SaveSettings() {
    HKEY hKey;
    DWORD disposition;
    
    if (RegCreateKeyEx(HKEY_CURRENT_USER, APP_REGISTRY_KEY, 0, NULL, 0,
                       KEY_WRITE, NULL, &hKey, &disposition) == ERROR_SUCCESS) {
        DWORD value;
        
        value = (DWORD)m_settings.timeFormat;
        RegSetValueEx(hKey, L"TimeFormat", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = m_settings.showSeconds ? 1 : 0;
        RegSetValueEx(hKey, L"ShowSeconds", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = m_settings.alwaysOnTop ? 1 : 0;
        RegSetValueEx(hKey, L"AlwaysOnTop", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = (DWORD)m_settings.textColor;
        RegSetValueEx(hKey, L"TextColor", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = (DWORD)m_settings.transparency;
        RegSetValueEx(hKey, L"Transparency", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = (DWORD)m_settings.fontSize;
        RegSetValueEx(hKey, L"FontSize", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = (DWORD)m_settings.windowX;
        RegSetValueEx(hKey, L"WindowX", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = (DWORD)m_settings.windowY;
        RegSetValueEx(hKey, L"WindowY", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = (DWORD)m_settings.windowWidth;
        RegSetValueEx(hKey, L"WindowWidth", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = (DWORD)m_settings.windowHeight;
        RegSetValueEx(hKey, L"WindowHeight", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = m_settings.clockVisible ? 1 : 0;
        RegSetValueEx(hKey, L"ClockVisible", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        RegCloseKey(hKey);
    }
}

void SettingsManager::ResetToDefaults() {
    InitializeDefaults();
    SaveSettings();
}

wchar_t* SettingsManager::GetExecutablePath() {
    static wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    return path;
}

bool SettingsManager::IsStartupEnabled() {
    HKEY hKey;
    bool enabled = false;
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, APP_STARTUP_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t value[MAX_PATH];
        DWORD size = sizeof(value);
        DWORD type;
        
        if (RegQueryValueEx(hKey, APP_REGISTRY_VALUE, NULL, &type, (LPBYTE)value, &size) == ERROR_SUCCESS) {
            if (type == REG_SZ) {
                enabled = true;
            }
        }
        
        RegCloseKey(hKey);
    }
    
    return enabled;
}

bool SettingsManager::SetStartupEnabled(bool enable) {
    HKEY hKey;
    bool success = false;
    LONG result;
    
    result = RegOpenKeyEx(HKEY_CURRENT_USER, APP_STARTUP_KEY, 0, KEY_WRITE, &hKey);
    
    if (enable) {
        if (result == ERROR_SUCCESS) {
            wchar_t* path = GetExecutablePath();
            DWORD size = (wcslen(path) + 1) * sizeof(wchar_t);
            
            if (RegSetValueEx(hKey, APP_REGISTRY_VALUE, 0, REG_SZ, (LPBYTE)path, size) == ERROR_SUCCESS) {
                success = true;
            }
            RegCloseKey(hKey);
        }
    } else {
        if (result == ERROR_SUCCESS) {
            LONG deleteResult = RegDeleteValue(hKey, APP_REGISTRY_VALUE);
            if (deleteResult == ERROR_SUCCESS || deleteResult == ERROR_FILE_NOT_FOUND) {
                success = true;
            }
            RegCloseKey(hKey);
        } else if (result == ERROR_FILE_NOT_FOUND) {
            success = true;
        }
    }
    
    return success;
}

// Made with Bob
