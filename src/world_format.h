#ifndef WORLD_FORMAT_H
#define WORLD_FORMAT_H

// Pure string/date helpers for the world clock row. Deliberately free of
// <windows.h> so they can be unit-tested with the host compiler.

#include <stddef.h>

const int WORLD_LABEL_MAX = 16;

// Share of the window height (percent) given to the main time when the
// world row is shown. User-adjustable within [MIN, MAX].
const int WORLD_SPLIT_MIN = 50;
const int WORLD_SPLIT_MAX = 90;
const int WORLD_SPLIT_DEFAULT = 70;

// Height in pixels of the main-time band: mainPercent of clientHeight,
// with mainPercent clamped to [WORLD_SPLIT_MIN, WORLD_SPLIT_MAX].
int SplitMainHeight(int clientHeight, int mainPercent);

// Short label from a Windows timezone display name: the text after the
// "(UTC+hh:mm)" prefix up to the first comma, trimmed and truncated to
// WORLD_LABEL_MAX characters.
void DeriveDefaultLabel(const wchar_t* displayName, wchar_t* out, size_t outLen);

// The trimmed custom label if non-empty, else DeriveDefaultLabel(displayName).
void ResolveLabel(const wchar_t* customLabel, const wchar_t* displayName,
                  wchar_t* out, size_t outLen);

// -1, 0 or +1: whether the zone's date is before, equal to or after local.
int DayOffset(int localY, int localM, int localD, int zoneY, int zoneM, int zoneD);

// "HH:MM Label" (or "H:MM AM/PM Label"), plus " +1" / " −1" when the
// zone's date differs from local.
void FormatWorldEntry(wchar_t* out, size_t outLen, const wchar_t* label,
                      int hour, int minute, bool use12h, int dayOffset);

// Joins the non-empty entries with a 4-space separator.
void JoinWorldEntries(const wchar_t* a, const wchar_t* b, wchar_t* out, size_t outLen);

// desiredPx if the text fits in availableWidth, else scaled down
// proportionally, never below minPx.
float FitFontToWidth(float desiredPx, float measuredWidth, float availableWidth, float minPx);

// Font size for the world row, derived from its band height (a share of
// the band, never below a readable minimum).
float WorldRowDesiredPx(float bandHeight);

// Final world row font size: WorldRowDesiredPx(bandHeight), shrunk so a row
// measuring measuredAtDesired at that size fits availableWidth. The width
// fit may go below the height minimum (down to a hard floor) so the row
// never needs to wrap.
float WorldRowFontPx(float bandHeight, float measuredAtDesired, float availableWidth);

#endif // WORLD_FORMAT_H
