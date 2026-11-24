#include "world_format.h"
#include <wchar.h>

// Copies [begin, end) into out with surrounding spaces trimmed and the
// result truncated to maxChars (and to the buffer size).
static void CopyTrimmed(const wchar_t* begin, const wchar_t* end,
                        size_t maxChars, wchar_t* out, size_t outLen) {
    if (outLen == 0) return;
    while (begin < end && *begin == L' ') begin++;
    while (end > begin && *(end - 1) == L' ') end--;

    size_t len = (size_t)(end - begin);
    if (len > maxChars) len = maxChars;
    if (len > outLen - 1) len = outLen - 1;
    wmemcpy(out, begin, len);
    out[len] = L'\0';

    // Truncation may leave a trailing space ("Foo Bar" -> "Foo ").
    while (len > 0 && out[len - 1] == L' ') out[--len] = L'\0';
}

void DeriveDefaultLabel(const wchar_t* displayName, wchar_t* out, size_t outLen) {
    const wchar_t* start = displayName;
    const wchar_t* paren = wcschr(displayName, L')');
    if (paren) start = paren + 1;

    const wchar_t* end = wcschr(start, L',');
    if (!end) end = start + wcslen(start);

    CopyTrimmed(start, end, WORLD_LABEL_MAX, out, outLen);
}

void ResolveLabel(const wchar_t* customLabel, const wchar_t* displayName,
                  wchar_t* out, size_t outLen) {
    CopyTrimmed(customLabel, customLabel + wcslen(customLabel),
                WORLD_LABEL_MAX, out, outLen);
    if (outLen > 0 && out[0] == L'\0') {
        DeriveDefaultLabel(displayName, out, outLen);
    }
}

int DayOffset(int localY, int localM, int localD, int zoneY, int zoneM, int zoneD) {
    long local = (long)localY * 10000 + localM * 100 + localD;
    long zone = (long)zoneY * 10000 + zoneM * 100 + zoneD;
    if (zone > local) return 1;
    if (zone < local) return -1;
    return 0;
}

void FormatWorldEntry(wchar_t* out, size_t outLen, const wchar_t* label,
                      int hour, int minute, bool use12h, int dayOffset) {
    const wchar_t* marker = L"";
    if (dayOffset > 0) marker = L" +1";
    else if (dayOffset < 0) marker = L" −1";

    if (use12h) {
        int h = hour % 12;
        if (h == 0) h = 12;
        swprintf(out, outLen, L"%d:%02d %ls %ls%ls",
                 h, minute, hour >= 12 ? L"PM" : L"AM", label, marker);
    } else {
        swprintf(out, outLen, L"%02d:%02d %ls%ls", hour, minute, label, marker);
    }
}

void JoinWorldEntries(const wchar_t* a, const wchar_t* b, wchar_t* out, size_t outLen) {
    if (a[0] && b[0]) {
        swprintf(out, outLen, L"%ls    %ls", a, b);
    } else {
        swprintf(out, outLen, L"%ls", a[0] ? a : b);
    }
}

float FitFontToWidth(float desiredPx, float measuredWidth, float availableWidth, float minPx) {
    float size = desiredPx;
    if (measuredWidth > availableWidth && measuredWidth > 0) {
        size = desiredPx * availableWidth / measuredWidth;
    }
    return size < minPx ? minPx : size;
}

// World row font size relative to its band height, the minimum
// height-derived size, and the hard floor when shrinking to fit the width.
static const float WORLD_FONT_SHARE = 0.60f;
static const float WORLD_FONT_MIN_PX = 8.0f;
static const float WORLD_FONT_FLOOR_PX = 4.0f;

float WorldRowDesiredPx(float bandHeight) {
    float desired = bandHeight * WORLD_FONT_SHARE;
    return desired < WORLD_FONT_MIN_PX ? WORLD_FONT_MIN_PX : desired;
}

float WorldRowFontPx(float bandHeight, float measuredAtDesired, float availableWidth) {
    return FitFontToWidth(WorldRowDesiredPx(bandHeight), measuredAtDesired,
                          availableWidth, WORLD_FONT_FLOOR_PX);
}

int SplitMainHeight(int clientHeight, int mainPercent) {
    if (mainPercent < WORLD_SPLIT_MIN) mainPercent = WORLD_SPLIT_MIN;
    if (mainPercent > WORLD_SPLIT_MAX) mainPercent = WORLD_SPLIT_MAX;
    return clientHeight * mainPercent / 100;
}
