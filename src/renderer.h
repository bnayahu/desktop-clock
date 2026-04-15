#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>
#include <gdiplus.h>
#include "settings.h"

using namespace Gdiplus;

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
