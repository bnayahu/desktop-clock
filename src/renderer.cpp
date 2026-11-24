#include "renderer.h"
#include "world_clock.h"
#include "world_format.h"
#include <stdio.h>

// Share of the client height given to the main time when the world row shows.
static const float MAIN_ROW_SHARE = 0.70f;
// Fraction of the window width the world row may occupy.
static const float WORLD_ROW_WIDTH_SHARE = 0.95f;

bool BuildWorldRow(const ClockSettings& s, const SYSTEMTIME& local,
                   wchar_t* out, size_t outLen) {
    wchar_t entries[WORLD_ZONE_COUNT][64];
    bool any = false;

    for (int i = 0; i < WORLD_ZONE_COUNT; i++) {
        entries[i][0] = L'\0';
        SYSTEMTIME zone;
        if (!GetZoneTime(s.worldTz[i], &zone)) continue;

        wchar_t displayName[128] = L"";
        GetZoneDisplayName(s.worldTz[i], displayName, 128);
        wchar_t label[WORLD_LABEL_MAX + 1];
        ResolveLabel(s.worldLabel[i], displayName, label, WORLD_LABEL_MAX + 1);

        int offset = DayOffset(local.wYear, local.wMonth, local.wDay,
                               zone.wYear, zone.wMonth, zone.wDay);
        FormatWorldEntry(entries[i], 64, label, zone.wHour, zone.wMinute,
                         s.timeFormat == TIME_12H, offset);
        any = true;
    }

    if (!any) return false;
    JoinWorldEntries(entries[0], entries[1], out, outLen);
    return true;
}

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
    
    // Offset into the target rect (a no-op for the full client rect).
    return RectF(x + rect->left, y + rect->top, boundingBox.Width, boundingBox.Height);
}

void ClockRenderer::Render(HDC hdc, const RECT* rect, const ClockSettings& settings) {
    if (!m_initialized) {
        return;
    }
    
    // Create Graphics object
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
    
    // Clear background with background transparency setting
    // This creates the transparent background while text uses its own alpha
    graphics.Clear(Color(settings.transparency, 0, 0, 0));
    
    // Get current time
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    // Format time string
    wchar_t timeStr[32];
    FormatTimeString(timeStr, 32, &st, settings);
    
    // With world clocks configured, the main time takes the top share of the
    // window and the world row the rest. Otherwise the layout is unchanged.
    wchar_t worldRow[160];
    bool showWorld = BuildWorldRow(settings, st, worldRow, 160);
    
    RECT mainRect = *rect;
    RECT worldRect = *rect;
    if (showWorld) {
        int clientHeight = rect->bottom - rect->top;
        mainRect.bottom = rect->top + (int)(clientHeight * MAIN_ROW_SHARE);
        worldRect.top = mainRect.bottom;
    }
    
    // Calculate font size based on (main row) size
    int fontSize = CalculateFontSize(&mainRect, settings.fontSize);
    
    // Create font with user-selected font name
    Font font(settings.fontName, (REAL)fontSize, FontStyleRegular, UnitPixel);
    
    // Create brush with color and text alpha (separate from background transparency)
    Color color(settings.textAlpha,
               GetRValue(settings.textColor),
               GetGValue(settings.textColor),
               GetBValue(settings.textColor));
    SolidBrush brush(color);
    
    // Center and draw text
    RectF layoutRect = CenterText(graphics, font, timeStr, &mainRect);
    graphics.DrawString(timeStr, -1, &font, layoutRect,
                       StringFormat::GenericDefault(), &brush);
    
    if (showWorld) {
        // Size the world row to its band, then shrink it to fit the width.
        float band = (float)(worldRect.bottom - worldRect.top);
        float desired = WorldRowDesiredPx(band);
        
        Font probe(settings.fontName, desired, FontStyleRegular, UnitPixel);
        RectF unbounded(0, 0, 100000.0f, 100000.0f);
        RectF measured;
        graphics.MeasureString(worldRow, -1, &probe, unbounded,
                              StringFormat::GenericDefault(), &measured);
        
        float available = (rect->right - rect->left) * WORLD_ROW_WIDTH_SHARE;
        float worldSize = WorldRowFontPx(band, measured.Width, available);
        
        // Single line, never wrapped: if it still can't fit at the floor
        // size it is trimmed with an ellipsis rather than spilling over.
        StringFormat noWrap(StringFormat::GenericDefault());
        noWrap.SetFormatFlags(noWrap.GetFormatFlags() | StringFormatFlagsNoWrap);
        noWrap.SetTrimming(StringTrimmingEllipsisCharacter);
        
        Font worldFont(settings.fontName, worldSize, FontStyleRegular, UnitPixel);
        RectF worldLayout = CenterText(graphics, worldFont, worldRow, &worldRect);
        graphics.DrawString(worldRow, -1, &worldFont, worldLayout, &noWrap, &brush);
    }
}

// Made with Bob
