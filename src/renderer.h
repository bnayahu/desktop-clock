#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>
#include <gdiplus.h>
#include "settings.h"

using namespace Gdiplus;

// Builds the world clock row text (e.g. "Tokyo 21:35 +1    NYC 08:35") for
// the configured zones. Zones with empty or unknown timezone keys are
// skipped. Returns false if no zone resolves.
bool BuildWorldRow(const ClockSettings& s, const SYSTEMTIME& local,
                   wchar_t* out, size_t outLen);

class ClockRenderer {
public:
    ClockRenderer();
    ~ClockRenderer();
    
    bool Initialize();
    void Render(HDC hdc, const RECT* rect, const ClockSettings& settings);
    
private:
    void FormatTimeString(wchar_t* buffer, size_t bufferSize, 
                         const SYSTEMTIME* st, const ClockSettings& settings);
    int CalculateFontSize(const RECT* rect, FontSize fontSize);
    RectF CenterText(Graphics& graphics, Font& font, 
                    const wchar_t* text, const RECT* rect);
    
    ULONG_PTR m_gdiplusToken;
    bool m_initialized;
};

#endif // RENDERER_H

// Made with Bob
