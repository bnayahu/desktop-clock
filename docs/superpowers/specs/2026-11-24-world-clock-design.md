# World Clock — Design

**Date:** 2026-11-24
**Size:** small change

## Intent

Track colleagues in other cities: answer "what time is it for them right now?" at a
glance, without leaving the desktop clock.

## Goals

- Optionally show up to 2 additional timezones on a row below the main time, in a
  smaller font.
- Each zone has a user-editable label (e.g. "Tokyo", "Dana").
- Show a day-offset marker when the zone's date differs from local (`+1` / `−1`).
- DST correct for every zone, via Windows' own timezone data.

## Non-goals

- More than 2 zones, per-zone colors/fonts, seconds on the secondary row, date display.

## Anti-goals

- With no zones configured, the clock must look and behave exactly as today.
- The window must never resize itself.

## Approaches

- **A. Split the existing window height (chosen).** When at least one zone is set, main
  time gets ~70% of the client height, the world row ~30%. Window size stays
  user-controlled; font still scales with window. Chosen because it preserves the
  existing "window size decides text size" rule and never moves/resizes the window.
- **B. Grow the window to fit.** Keeps main digits the same size, but the window changes
  size on its own, complicating saved size, resizing, and screen-edge cases. Rejected.

## Decided behavior

- **Layout:** world row centered beneath main time; both zones side by side on one row,
  separated by spacing, e.g. `21:35 Tokyo +1    08:35 NYC`. Only configured zones appear.
- **Format:** follows the main clock's 12/24h setting; never shows seconds.
- **Style:** same font family, text color, and text opacity as the main time.
- **Label:** user-editable; if blank, a short default derived from the zone's display name.
- **Enabling:** a zone appears only once a timezone is selected (`(None)` = off).

## Revisions (2026-11-24, after first Windows trial)

- The 70/30 split is user-adjustable: a "Main time size" slider, 50–90%, default 70%.
- Entries read time first, then label: `10:30 CT`, `9:35 PM Tokyo +1`.

## Settings / persistence

New registry values under `HKCU\Software\DesktopClock`:

| Value | Contents |
| --- | --- |
| `WorldTz1`, `WorldTz2` | Windows timezone key name (e.g. `Tokyo Standard Time`); empty = off |
| `WorldLabel1`, `WorldLabel2` | Custom label; empty = use default |
| `WorldSplit` | Main time share of height in percent, 50–90 (default 70) |

Missing values = off, so existing installs are unchanged.

## Settings dialog

New "World Clock" group with two rows, each: timezone dropdown (first entry `(None)`,
then the system list in display-name order) + label edit box. Dialog grows to fit.
OK/Apply/Cancel semantics as for existing settings.

## Left to the builder

- APIs: `EnumDynamicTimeZoneInformation` to list zones and
  `SystemTimeToTzSpecificLocalTimeEx` to convert (both available on Win 10/11).
- World-row font size and spacing between zones.
- Default-label derivation rule (e.g. first city after the `(UTC±hh:mm)` prefix) and
  max label length (suggest 16 chars).
- Whether the dialog preview also shows the world row.
- README updates: feature list, settings list, registry table, changelog entry.
