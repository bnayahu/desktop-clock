# World Clock Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Optionally show up to 2 labeled timezones on a smaller row beneath the main clock time.

**Architecture:** Pure, Windows-free formatting helpers (`world_format`) carry all string/date logic and are unit-tested with the host `g++`. A thin Win32 module (`world_clock`) lists timezones and converts the current time. Settings gain 4 registry values; the renderer splits the client rect 70/30 when any zone is set; the settings dialog gains a "World Clock" group.

**Tech Stack:** C++11, Win32, GDI+, MinGW-w64 cross-compile (`build.sh`); host `g++` for unit tests (no Wine available).

**Spec:** `docs/superpowers/specs/2026-11-24-world-clock-design.md`

## Global Constraints

- Target Windows 10/11 x64; MinGW default `_WIN32_WINNT` is already `0x0A00`, so `EnumDynamicTimeZoneInformation` / `SystemTimeToTzSpecificLocalTimeEx` are available with no extra defines.
- C++11 only (`-std=c++11`), follow existing code style (4-space indent, `m_` members, `// Made with Bob` trailer is existing, do not add to new files).
- With no zone configured, rendering and window behavior must be identical to today. The window never resizes itself.
- World row: same font family, text color, text alpha as main; follows 12/24h; never shows seconds.
- Day marker: `+1` / `−1` (U+2212 minus sign) when the zone's date differs from local; nothing when equal.
- Registry values: `WorldTz1`, `WorldTz2` (REG_SZ, Windows timezone key name, empty = off), `WorldLabel1`, `WorldLabel2` (REG_SZ, empty = default label).
- Label max length: 16 characters.
- `world_format.*` must not include `<windows.h>`; in wide format strings use `%ls` for wide-string args (portable between glibc and msvcrt).
- Every commit: `git commit -S -s` with trailer `Assisted-By: Claude (Anthropic AI) <noreply@anthropic.com>`. No `Co-Authored-By`. If signing fails, stop and report.

## Review Focus

1. **Long labels / narrow window:** two 16-char labels on a 200px-wide window must shrink the world-row font to fit the width, never clip or overflow. → `FitFontToWidth` test in Task 1.
2. **Only zone 2 set:** row shows only zone 2 with no leading separator or gap. → `JoinWorldEntries` test in Task 1.
3. **Date boundaries:** Dec 31 local vs Jan 1 zone (and month ends, leap day) yields `+1`/`−1`, not wrong sign. → `DayOffset` tests in Task 1.
4. **Blank or whitespace-only label:** falls back to the derived default label. → `ResolveLabel` test in Task 1.
5. **Stored timezone key no longer valid** (hand-edited registry, renamed zone): that zone is silently treated as off in the clock, and the dialog shows `(None)` for it. → `GetZoneTime` returns `false`, covered by Task 2 Step 4 manual check and Task 3 rendering skip.

---

### Task 1: Pure world-row formatting helpers + host test harness

**Files:**
- Create: `src/world_format.h`, `src/world_format.cpp`
- Create: `tests/test_world_format.cpp`, `tests/run_tests.sh`
- Modify: `.gitignore` (ignore `tests/test_world_format` binary)

**Interfaces:**
- Produces (all in `world_format.h`, no Windows types):
  - `const int WORLD_LABEL_MAX = 16;`
  - `void DeriveDefaultLabel(const wchar_t* displayName, wchar_t* out, size_t outLen);` — text after the first `)` (skip spaces), up to the first `,`, trimmed, truncated to `WORLD_LABEL_MAX`. No `)` → whole string trimmed/truncated.
  - `void ResolveLabel(const wchar_t* customLabel, const wchar_t* displayName, wchar_t* out, size_t outLen);` — trimmed custom label if non-empty, else `DeriveDefaultLabel`.
  - `int DayOffset(int localY, int localM, int localD, int zoneY, int zoneM, int zoneD);` — returns -1, 0 or +1 by comparing dates.
  - `void FormatWorldEntry(wchar_t* out, size_t outLen, const wchar_t* label, int hour, int minute, bool use12h, int dayOffset);`
  - `void JoinWorldEntries(const wchar_t* a, const wchar_t* b, wchar_t* out, size_t outLen);` — joins non-empty entries with 4 spaces.
  - `float FitFontToWidth(float desiredPx, float measuredWidth, float availableWidth, float minPx);` — returns `desiredPx` if it fits, else `desiredPx * available / measured`, never below `minPx`.

- [ ] **Step 1: Write the failing tests** in `tests/test_world_format.cpp` — a minimal `CHECK(cond)` / `CHECK_WSTR(actual, expected)` macro pair counting failures, `main` returns nonzero on any failure. Cases:

```cpp
// DeriveDefaultLabel
"(UTC+09:00) Osaka, Sapporo, Tokyo" -> L"Osaka"
"(UTC-05:00) Eastern Time (US & Canada)" -> L"Eastern Time (US" // truncated at 16
"(UTC) Coordinated Universal Time" -> L"Coordinated Univ"
"Plain Name" -> L"Plain Name"
// ResolveLabel
custom L"  Dana " , any display -> L"Dana"
custom L"   ", "(UTC+01:00) Amsterdam, Berlin" -> L"Amsterdam"
custom L"" , same -> L"Amsterdam"
// DayOffset
(2026,11,24, 2026,11,24) == 0
(2026,11,24, 2026,11,25) == 1
(2026,11,24, 2026,11,23) == -1
(2026,12,31, 2027,1,1)  == 1
(2027,1,1,  2026,12,31) == -1
(2028,2,29, 2028,3,1)   == 1
// FormatWorldEntry
(L"Tokyo", 9, 35, false, 1)  -> L"Tokyo 09:35 +1"
(L"NYC",  8, 5,  false, 0)   -> L"NYC 08:05"
(L"LA",   23, 0, false, -1)  -> L"LA 23:00 −1"
(L"Tokyo", 0, 15, true, 0)   -> L"Tokyo 12:15 AM"
(L"Tokyo", 12, 0, true, 0)   -> L"Tokyo 12:00 PM"
(L"Tokyo", 21, 35, true, 1)  -> L"Tokyo 9:35 PM +1"
// JoinWorldEntries
(L"A 1", L"B 2") -> L"A 1    B 2"
(L"",    L"B 2") -> L"B 2"
(L"A 1", L"")    -> L"A 1"
(L"",    L"")    -> L""
// FitFontToWidth
(20, 100, 200, 8) == 20
(20, 400, 200, 8) == 10
(20, 4000, 200, 8) == 8
```

`tests/run_tests.sh`: `set -e`; `g++ -std=c++11 -Wall -Wextra -o tests/test_world_format tests/test_world_format.cpp src/world_format.cpp && ./tests/test_world_format`. Make executable.

- [ ] **Step 2: Run to verify it fails**

Run: `./tests/run_tests.sh`
Expected: compile failure (`world_format.h: No such file`).

- [ ] **Step 3: Implement the functions in `src/world_format.{h,cpp}`** per the Interfaces block. Use `swprintf` with explicit size and `%ls`; 12h hour rules match `renderer.cpp` (`0→12 AM`, `12→12 PM`). `DayOffset` compares `y*10000+m*100+d` and clamps to sign.

- [ ] **Step 4: Run to verify it passes**

Run: `./tests/run_tests.sh`
Expected: prints a pass summary, exit code 0.

- [ ] **Step 5: Confirm it also cross-compiles** — add `src/world_format.cpp` to the source list in `build.sh`, run `./build.sh`.
Expected: `Build successful!` with no new warnings.

- [ ] **Step 6: Commit**

```bash
git add src/world_format.h src/world_format.cpp tests/ build.sh .gitignore
git commit -S -s -m "feat: add world clock formatting helpers with host tests

Assisted-By: Claude (Anthropic AI) <noreply@anthropic.com>"
```

---

### Task 2: Timezone access + settings persistence

**Files:**
- Create: `src/world_clock.h`, `src/world_clock.cpp`
- Modify: `src/settings.h` (`ClockSettings`), `src/settings.cpp` (defaults, `LoadSettings`, `SaveSettings`)
- Modify: `build.sh` (add `src/world_clock.cpp`)

**Interfaces:**
- Produces in `settings.h`:
  - `const int WORLD_ZONE_COUNT = 2;` and `const int WORLD_TZ_KEY_LEN = 128;`
  - `ClockSettings` fields: `wchar_t worldTz[WORLD_ZONE_COUNT][WORLD_TZ_KEY_LEN];` and `wchar_t worldLabel[WORLD_ZONE_COUNT][WORLD_LABEL_MAX + 1];` (include `world_format.h` for the constant). Defaults: all empty strings.
  - `bool HasWorldZones(const ClockSettings& s);` — true if any `worldTz[i][0] != 0`.
- Produces in `world_clock.h`:
  - `struct TimeZoneEntry { wchar_t keyName[128]; wchar_t displayName[128]; LONG bias; };`
  - `int EnumerateTimeZones(TimeZoneEntry* out, int maxCount);` — via `EnumDynamicTimeZoneInformation`, sorted by UTC offset west→east (descending `Bias`), then display name. Returns count.
  - `bool GetZoneTime(const wchar_t* keyName, SYSTEMTIME* zoneTime);` — finds the zone by key name, converts `GetSystemTime()` with `SystemTimeToTzSpecificLocalTimeEx`. Returns `false` for empty/unknown key. Cache the found `DYNAMIC_TIME_ZONE_INFORMATION` per key (small static cache keyed by name) so per-second repaints don't re-enumerate.
  - `bool GetZoneDisplayName(const wchar_t* keyName, wchar_t* out, size_t outLen);`

- [ ] **Step 1: Add the settings fields and load/save.** Load each value as REG_SZ into a local buffer sized to the field, accept only `type == REG_SZ`, force null termination at the last element, then copy. Save with `RegSetValueEx(..., REG_SZ, ..., (wcslen+1)*sizeof(wchar_t))`, mirroring `FontName`. Truncate loaded labels to `WORLD_LABEL_MAX`.

- [ ] **Step 2: Implement `world_clock.cpp`** per the Interfaces block; add to `build.sh`.

- [ ] **Step 3: Build**

Run: `./build.sh && ./tests/run_tests.sh`
Expected: `Build successful!`, no new warnings; unit tests still pass.

- [ ] **Step 4: Commit**

```bash
git add src/world_clock.h src/world_clock.cpp src/settings.h src/settings.cpp build.sh
git commit -S -s -m "feat: add timezone lookup and world clock settings

Assisted-By: Claude (Anthropic AI) <noreply@anthropic.com>"
```

---

### Task 3: Render the world row

**Files:**
- Modify: `src/renderer.h`, `src/renderer.cpp`

**Interfaces:**
- Consumes: `HasWorldZones`, `GetZoneTime`, `GetZoneDisplayName`, `ResolveLabel`, `DayOffset`, `FormatWorldEntry`, `JoinWorldEntries`, `FitFontToWidth`.
- Produces (free function declared in `renderer.h`, reused by Task 4's preview): `bool BuildWorldRow(const ClockSettings& s, const SYSTEMTIME& local, wchar_t* out, size_t outLen);` — builds the joined row text; zones with empty or unknown keys are skipped; returns false if no zone resolves.

- [ ] **Step 1: Make `CenterText` respect the rect's origin** (add `rect->left`/`rect->top` to the returned x/y). For the full client rect (origin 0,0) output is unchanged.

- [ ] **Step 2: Split layout in `Render`.** If `BuildWorldRow` returns true: main rect = top 70% of client height, world rect = remaining 30%; main font from `CalculateFontSize(&mainRect, ...)` (so it scales as today, just from a shorter rect). World font px = `worldRectHeight * 0.6`, then measure the row at that size and apply `FitFontToWidth(desired, measured, clientWidth * 0.95, 8)`. Draw both with the same brush/font family. If false: render exactly as today with the full rect.

- [ ] **Step 3: Build**

Run: `./build.sh && ./tests/run_tests.sh`
Expected: success, no new warnings.

- [ ] **Step 4: Commit**

```bash
git add src/renderer.h src/renderer.cpp
git commit -S -s -m "feat: draw world clock row beneath main time

Assisted-By: Claude (Anthropic AI) <noreply@anthropic.com>"
```

---

### Task 4: Settings dialog, preview, docs, version

**Files:**
- Modify: `resources/resource.h`, `resources/clock.rc`, `src/main.cpp`, `resources/app_strings.h`, `README.md`

**Interfaces:**
- Consumes: `EnumerateTimeZones`, `TimeZoneEntry`, `BuildWorldRow`, `WORLD_LABEL_MAX`, settings fields from Task 2.
- New IDs: `IDC_COMBO_TZ1 2016`, `IDC_EDIT_LABEL1 2017`, `IDC_COMBO_TZ2 2018`, `IDC_EDIT_LABEL2 2019`.

- [ ] **Step 1: Dialog layout.** In `IDD_SETTINGS`, insert a `GROUPBOX "World Clock"` (height ~55) between Startup and Preview; two rows each: `LTEXT "Zone 1:"`, `COMBOBOX IDC_COMBO_TZ1` (`CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP`, ~150 wide, dropdown height ~150), `LTEXT "Label:"`, `EDITTEXT IDC_EDIT_LABEL1` (`ES_AUTOHSCROLL`, ~70 wide). Same for zone 2. Shift Preview and OK/Cancel/Apply down and grow dialog height accordingly; make the Preview static two lines tall (~30).

- [ ] **Step 2: Wire the dialog in `main.cpp`.** On `WM_INITDIALOG`: enumerate zones once into a static array; fill each combo with `(None)` (item data -1) then display names (item data = array index); select the entry whose `keyName` matches `tempSettings.worldTz[i]`, else `(None)` (covers invalid stored keys). Set label edits from settings and `EM_LIMITTEXT` to `WORLD_LABEL_MAX`. On `CBN_SELCHANGE` / `EN_CHANGE`: update `tempSettings.worldTz[i]` (empty for `(None)`) / `worldLabel[i]` and call `UpdatePreview`.

- [ ] **Step 3: Preview.** In `UpdatePreview`, if `BuildWorldRow(*settings, st, ...)` returns true, set the preview text to `mainTime + L"\r\n" + worldRow`.

- [ ] **Step 4: Version and docs.** Bump `APP_VERSION_MINOR` to `3`. README: feature bullet ("World clock — up to two extra timezones with custom labels, shown on a smaller row; `+1`/`−1` when their date differs"), settings-dialog bullet, the 4 registry rows from Global Constraints, and a `### v1.3.0` changelog entry; remove nothing else. Add "World clock" to About dialog feature list only if it fits without resizing (optional).

- [ ] **Step 5: Build and test**

Run: `./build.sh && ./tests/run_tests.sh`
Expected: success, no new warnings.

- [ ] **Step 6: Commit**

```bash
git add resources/ src/main.cpp README.md
git commit -S -s -m "feat: add world clock settings to dialog

Assisted-By: Claude (Anthropic AI) <noreply@anthropic.com>"
```

- [ ] **Step 7: Manual check on Windows (by the user — no Wine here).** Copy `DesktopClock.exe` to Windows and verify:
  1. Fresh/no zones: clock looks exactly as v1.2.
  2. Zone 1 = Tokyo, label blank → row shows `Osaka 21:35 +1` style default; set label "Tokyo" → updates on Apply.
  3. Only zone 2 set → single entry, centered.
  4. Two 16-char labels, window dragged narrow → row shrinks, no clipping.
  5. 12h mode → world row uses AM/PM; seconds on → world row still has no seconds.
  6. Set `WorldTz1` to `Bogus Zone` in regedit, restart → zone hidden, dialog shows `(None)`.
  7. A half-hour zone (India Standard Time) shows the correct :30 offset.
