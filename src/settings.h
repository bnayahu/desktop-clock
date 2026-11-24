#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>
#include "world_format.h"

const int WORLD_ZONE_COUNT = 2;
const int WORLD_TZ_KEY_LEN = 128;   // matches DYNAMIC_TIME_ZONE_INFORMATION::TimeZoneKeyName

enum TimeFormat {
    TIME_12H = 0,
    TIME_24H = 1
};

enum FontSize {
    FONT_SIZE_SMALL = 0,
    FONT_SIZE_MEDIUM = 1,
    FONT_SIZE_LARGE = 2
};

struct ClockSettings {
    TimeFormat timeFormat;
    bool showSeconds;
    bool alwaysOnTop;
    COLORREF textColor;
    BYTE transparency;          // Background/window transparency
    BYTE textAlpha;             // Text transparency (separate from background)
    FontSize fontSize;
    wchar_t fontName[32];       // Font family name
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;
    bool startWithWindows;
    bool clockVisible;
    wchar_t worldTz[WORLD_ZONE_COUNT][WORLD_TZ_KEY_LEN];        // Timezone key names; empty = off
    wchar_t worldLabel[WORLD_ZONE_COUNT][WORLD_LABEL_MAX + 1];  // Custom labels; empty = default
};

// True if at least one world clock zone is configured.
bool HasWorldZones(const ClockSettings& s);

class SettingsManager {
public:
    SettingsManager();
    ~SettingsManager();
    
    void LoadSettings();
    void SaveSettings();
    void ResetToDefaults();
    
    ClockSettings& GetSettings() { return m_settings; }
    const ClockSettings& GetSettings() const { return m_settings; }
    
    bool IsStartupEnabled();
    bool SetStartupEnabled(bool enable);
    
private:
    ClockSettings m_settings;
    void InitializeDefaults();
    wchar_t* GetExecutablePath();
};

#endif // SETTINGS_H

// Made with Bob
