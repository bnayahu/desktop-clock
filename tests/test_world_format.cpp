// Host-side unit tests for world_format (no Windows dependencies).
#include "../src/world_format.h"
#include <stdio.h>
#include <wchar.h>

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond) do { \
    g_checks++; \
    if (!(cond)) { \
        g_failures++; \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_WSTR(actual, expected) do { \
    g_checks++; \
    if (wcscmp((actual), (expected)) != 0) { \
        g_failures++; \
        printf("FAIL %s:%d: got \"%ls\", expected \"%ls\"\n", \
               __FILE__, __LINE__, (actual), (expected)); \
    } \
} while (0)

static void TestDeriveDefaultLabel() {
    wchar_t out[64];
    DeriveDefaultLabel(L"(UTC+09:00) Osaka, Sapporo, Tokyo", out, 64);
    CHECK_WSTR(out, L"Osaka");
    DeriveDefaultLabel(L"(UTC-05:00) Eastern Time (US & Canada)", out, 64);
    CHECK_WSTR(out, L"Eastern Time (US");
    DeriveDefaultLabel(L"(UTC) Coordinated Universal Time", out, 64);
    CHECK_WSTR(out, L"Coordinated Univ");
    DeriveDefaultLabel(L"Plain Name", out, 64);
    CHECK_WSTR(out, L"Plain Name");
}

static void TestResolveLabel() {
    wchar_t out[64];
    ResolveLabel(L"  Dana ", L"(UTC+09:00) Osaka, Sapporo, Tokyo", out, 64);
    CHECK_WSTR(out, L"Dana");
    ResolveLabel(L"   ", L"(UTC+01:00) Amsterdam, Berlin", out, 64);
    CHECK_WSTR(out, L"Amsterdam");
    ResolveLabel(L"", L"(UTC+01:00) Amsterdam, Berlin", out, 64);
    CHECK_WSTR(out, L"Amsterdam");
}

static void TestDayOffset() {
    CHECK(DayOffset(2026, 11, 24, 2026, 11, 24) == 0);
    CHECK(DayOffset(2026, 11, 24, 2026, 11, 25) == 1);
    CHECK(DayOffset(2026, 11, 24, 2026, 11, 23) == -1);
    CHECK(DayOffset(2026, 12, 31, 2027, 1, 1) == 1);
    CHECK(DayOffset(2027, 1, 1, 2026, 12, 31) == -1);
    CHECK(DayOffset(2028, 2, 29, 2028, 3, 1) == 1);
}

static void TestFormatWorldEntry() {
    wchar_t out[64];
    FormatWorldEntry(out, 64, L"Tokyo", 9, 35, false, 1);
    CHECK_WSTR(out, L"Tokyo 09:35 +1");
    FormatWorldEntry(out, 64, L"NYC", 8, 5, false, 0);
    CHECK_WSTR(out, L"NYC 08:05");
    FormatWorldEntry(out, 64, L"LA", 23, 0, false, -1);
    CHECK_WSTR(out, L"LA 23:00 −1");
    FormatWorldEntry(out, 64, L"Tokyo", 0, 15, true, 0);
    CHECK_WSTR(out, L"Tokyo 12:15 AM");
    FormatWorldEntry(out, 64, L"Tokyo", 12, 0, true, 0);
    CHECK_WSTR(out, L"Tokyo 12:00 PM");
    FormatWorldEntry(out, 64, L"Tokyo", 21, 35, true, 1);
    CHECK_WSTR(out, L"Tokyo 9:35 PM +1");
}

static void TestJoinWorldEntries() {
    wchar_t out[64];
    JoinWorldEntries(L"A 1", L"B 2", out, 64);
    CHECK_WSTR(out, L"A 1    B 2");
    JoinWorldEntries(L"", L"B 2", out, 64);
    CHECK_WSTR(out, L"B 2");
    JoinWorldEntries(L"A 1", L"", out, 64);
    CHECK_WSTR(out, L"A 1");
    JoinWorldEntries(L"", L"", out, 64);
    CHECK_WSTR(out, L"");
}

static void TestFitFontToWidth() {
    CHECK(FitFontToWidth(20, 100, 200, 8) == 20);
    CHECK(FitFontToWidth(20, 400, 200, 8) == 10);
    CHECK(FitFontToWidth(20, 4000, 200, 8) == 8);
}

int main() {
    TestDeriveDefaultLabel();
    TestResolveLabel();
    TestDayOffset();
    TestFormatWorldEntry();
    TestJoinWorldEntries();
    TestFitFontToWidth();
    printf("%d/%d checks passed\n", g_checks - g_failures, g_checks);
    return g_failures == 0 ? 0 : 1;
}
