#ifndef WORLD_FORMAT_H
#define WORLD_FORMAT_H

// Pure string/date helpers for the world clock row. Deliberately free of
// <windows.h> so they can be unit-tested with the host compiler.

#include <stddef.h>

const int WORLD_LABEL_MAX = 16;

// Short label from a Windows timezone display name: the text after the
// "(UTC+hh:mm)" prefix up to the first comma, trimmed and truncated to
// WORLD_LABEL_MAX characters.
void DeriveDefaultLabel(const wchar_t* displayName, wchar_t* out, size_t outLen);

// The trimmed custom label if non-empty, else DeriveDefaultLabel(displayName).
void ResolveLabel(const wchar_t* customLabel, const wchar_t* displayName,
                  wchar_t* out, size_t outLen);

// -1, 0 or +1: whether the zone's date is before, equal to or after local.
int DayOffset(int localY, int localM, int localD, int zoneY, int zoneM, int zoneD);

// "Label HH:MM" (or "Label H:MM AM/PM"), plus " +1" / " −1" when the
// zone's date differs from local.
void FormatWorldEntry(wchar_t* out, size_t outLen, const wchar_t* label,
                      int hour, int minute, bool use12h, int dayOffset);

// Joins the non-empty entries with a 4-space separator.
void JoinWorldEntries(const wchar_t* a, const wchar_t* b, wchar_t* out, size_t outLen);

// desiredPx if the text fits in availableWidth, else scaled down
// proportionally, never below minPx.
float FitFontToWidth(float desiredPx, float measuredWidth, float availableWidth, float minPx);

#endif // WORLD_FORMAT_H
