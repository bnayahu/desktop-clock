#include "world_clock.h"
#include <stdlib.h>
#include <wchar.h>

namespace {

// Small cache so per-second repaints don't re-enumerate every timezone.
// Unknown keys are cached too (found == false), so a bad stored key costs
// one enumeration rather than one per repaint.
struct CachedZone {
    wchar_t keyName[128];
    bool found;
    DYNAMIC_TIME_ZONE_INFORMATION info;
};

const int CACHE_SIZE = 4;
CachedZone g_cache[CACHE_SIZE];
int g_cacheCount = 0;
int g_cacheNext = 0;

bool FindZone(const wchar_t* keyName, DYNAMIC_TIME_ZONE_INFORMATION* info) {
    if (!keyName || keyName[0] == L'\0') return false;

    for (int i = 0; i < g_cacheCount; i++) {
        if (wcscmp(g_cache[i].keyName, keyName) == 0) {
            if (g_cache[i].found) *info = g_cache[i].info;
            return g_cache[i].found;
        }
    }

    bool found = false;
    DYNAMIC_TIME_ZONE_INFORMATION dtzi;
    for (DWORD i = 0; EnumDynamicTimeZoneInformation(i, &dtzi) == ERROR_SUCCESS; i++) {
        if (wcscmp(dtzi.TimeZoneKeyName, keyName) == 0) {
            found = true;
            *info = dtzi;
            break;
        }
    }

    CachedZone& slot = g_cache[g_cacheNext];
    wcsncpy(slot.keyName, keyName, 127);
    slot.keyName[127] = L'\0';
    slot.found = found;
    if (found) slot.info = *info;
    g_cacheNext = (g_cacheNext + 1) % CACHE_SIZE;
    if (g_cacheCount < CACHE_SIZE) g_cacheCount++;

    return found;
}

int CompareEntries(const void* a, const void* b) {
    const TimeZoneEntry* x = (const TimeZoneEntry*)a;
    const TimeZoneEntry* y = (const TimeZoneEntry*)b;
    // Larger bias = further west = earlier in the list.
    if (x->bias != y->bias) return x->bias > y->bias ? -1 : 1;
    return wcscmp(x->displayName, y->displayName);
}

} // namespace

int EnumerateTimeZones(TimeZoneEntry* out, int maxCount) {
    int count = 0;
    DYNAMIC_TIME_ZONE_INFORMATION dtzi;
    for (DWORD i = 0; count < maxCount &&
         EnumDynamicTimeZoneInformation(i, &dtzi) == ERROR_SUCCESS; i++) {
        TimeZoneEntry& e = out[count++];
        wcsncpy(e.keyName, dtzi.TimeZoneKeyName, 127);
        e.keyName[127] = L'\0';
        // DYNAMIC_TIME_ZONE_INFORMATION has no display name; it lives in
        // the zone's registry key.
        e.displayName[0] = L'\0';
        wchar_t subKey[256];
        swprintf(subKey, 256,
                 L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\%ls",
                 dtzi.TimeZoneKeyName);
        DWORD size = sizeof(e.displayName) - sizeof(wchar_t);
        if (RegGetValue(HKEY_LOCAL_MACHINE, subKey, L"Display", RRF_RT_REG_SZ,
                        NULL, e.displayName, &size) != ERROR_SUCCESS) {
            wcsncpy(e.displayName, dtzi.TimeZoneKeyName, 127);
        }
        e.displayName[127] = L'\0';
        e.bias = dtzi.Bias;
    }
    qsort(out, count, sizeof(TimeZoneEntry), CompareEntries);
    return count;
}

bool GetZoneTime(const wchar_t* keyName, SYSTEMTIME* zoneTime) {
    DYNAMIC_TIME_ZONE_INFORMATION info;
    if (!FindZone(keyName, &info)) return false;

    SYSTEMTIME utc;
    GetSystemTime(&utc);
    return SystemTimeToTzSpecificLocalTimeEx(&info, &utc, zoneTime) != FALSE;
}

bool GetZoneDisplayName(const wchar_t* keyName, wchar_t* out, size_t outLen) {
    DYNAMIC_TIME_ZONE_INFORMATION info;
    if (!FindZone(keyName, &info) || outLen == 0) return false;

    wchar_t subKey[256];
    swprintf(subKey, 256,
             L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\%ls",
             keyName);
    DWORD size = (DWORD)(outLen * sizeof(wchar_t));
    if (RegGetValue(HKEY_LOCAL_MACHINE, subKey, L"Display", RRF_RT_REG_SZ,
                    NULL, out, &size) != ERROR_SUCCESS) {
        wcsncpy(out, keyName, outLen - 1);
        out[outLen - 1] = L'\0';
    }
    return true;
}
