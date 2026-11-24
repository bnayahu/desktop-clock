#ifndef WORLD_CLOCK_H
#define WORLD_CLOCK_H

#include <windows.h>

// Win32 timezone access for the world clock row.

struct TimeZoneEntry {
    wchar_t keyName[128];       // Stable registry key, e.g. "Tokyo Standard Time"
    wchar_t displayName[128];   // e.g. "(UTC+09:00) Osaka, Sapporo, Tokyo"
    LONG bias;                  // Minutes west of UTC (Windows convention)
};

// Fills out with up to maxCount system timezones, ordered west to east by
// UTC offset, then by display name. Returns the number written.
int EnumerateTimeZones(TimeZoneEntry* out, int maxCount);

// Converts the current UTC time to the zone named by keyName.
// Returns false if keyName is empty or not a known timezone.
bool GetZoneTime(const wchar_t* keyName, SYSTEMTIME* zoneTime);

// Copies the zone's display name into out. Returns false if unknown.
bool GetZoneDisplayName(const wchar_t* keyName, wchar_t* out, size_t outLen);

#endif // WORLD_CLOCK_H
