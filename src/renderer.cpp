#include "renderer.h"
#include <stdio.h>

ClockRenderer::ClockRenderer() : m_gdiplusToken(0), m_initialized(false) {
}

ClockRenderer::~ClockRenderer() {
    if (m_initialized) {
        GdiplusShutdown(m_gdiplusToken);
    }
}

bool ClockRenderer::Initialize() {
    if (m_initialized) {
        return true;
    }
    
    GdiplusStartupInput gdiplusStartupInput;
    Status status = GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
    
    if (status == Ok) {
        m_initialized = true;
        return true;
    }
    
    return false;
}

void ClockRenderer::FormatTimeString(wchar_t* buffer, size_t bufferSize, 
                                     const SYSTEMTIME* st, const ClockSettings& settings) {
    if (settings.timeFormat == TIME_12H) {
        // 12-hour format with AM/PM
        int hour = st->wHour % 12;
        if (hour == 0) hour = 12;
        
        if (settings.showSeconds) {
            swprintf(buffer, bufferSize, L"%d:%02d:%02d %s", 
                    hour, st->wMinute, st->wSecond,
                    st->wHour >= 12 ? L"PM" : L"AM");
        } else {
            swprintf(buffer, bufferSize, L"%d:%02d %s",
                    hour, st->wMinute,
                    st->wHour >= 12 ? L"PM" : L"AM");
        }
    } else {
        // 24-hour format
        if (settings.showSeconds) {
            swprintf(buffer, bufferSize, L"%02d:%02d:%02d",
                    st->wHour, st->wMinute, st->wSecond);
        } else {
            swprintf(buffer, bufferSize, L"%02d:%02d",
                    st->wHour, st->wMinute);
        }
    }
}

int ClockRenderer::CalculateFontSize(const RECT* rect, FontSize fontSize) {
    // Base sizes for each font size setting
    int baseSizes[] = { 24, 36, 48 }; // Small, Medium, Large
    int baseSize = baseSizes[fontSize];
    
    // Calculate window height
    int windowHeight = rect->bottom - rect->top;
    
    // Scale font based on window height
    // Base calculation assumes 80 pixel height for base size
    int scaledSize = (windowHeight * baseSize) / 80;
    
    // Clamp to reasonable bounds
    if (scaledSize < 16) scaledSize = 16;
    if (scaledSize > 200) scaledSize = 200;
    
    return scaledSize;
}

RectF ClockRenderer::CenterText(Graphics& graphics, Font& font, 
                                const wchar_t* text, const RECT* rect) {
    // Measure the text
    RectF layoutRect(0, 0, (REAL)(rect->right - rect->left), 
                     (REAL)(rect->bottom - rect->top));
    RectF boundingBox;
    
    graphics.MeasureString(text, -1, &font, layoutRect, 
                          StringFormat::GenericDefault(), &boundingBox);
    
    // Calculate centered position
    REAL x = ((REAL)(rect->right - rect->left) - boundingBox.Width) / 2.0f;
    REAL y = ((REAL)(rect->bottom - rect->top) - boundingBox.Height) / 2.0f;
    
    // Ensure non-negative coordinates
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    
    return RectF(x, y, boundingBox.Width, boundingBox.Height);
}

void ClockRenderer::Render(HDC hdc, const RECT* rect, const ClockSettings& settings) {
    if (!m_initialized) {
        return;
    }
    
    // Create Graphics object
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
    
    // Clear background (transparent)
    graphics.Clear(Color(0, 0, 0, 0));
    
    // Get current time
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    // Format time string
    wchar_t timeStr[32];
    FormatTimeString(timeStr, 32, &st, settings);
    
    // Calculate font size based on window size
    int fontSize = CalculateFontSize(rect, settings.fontSize);
    
    // Create font
    Font font(L"Segoe UI", (REAL)fontSize, FontStyleRegular, UnitPixel);
    
    // Create brush with color and transparency
    Color color(settings.transparency,
               GetRValue(settings.textColor),
               GetGValue(settings.textColor),
               GetBValue(settings.textColor));
    SolidBrush brush(color);
    
    // Center and draw text
    RectF layoutRect = CenterText(graphics, font, timeStr, rect);
    graphics.DrawString(timeStr, -1, &font, layoutRect,
                       StringFormat::GenericDefault(), &brush);
}

// Made with Bob
