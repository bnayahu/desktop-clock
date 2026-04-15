#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>

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
    BYTE transparency;
    FontSize fontSize;
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;
    bool startWithWindows;
    bool clockVisible;
};

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
