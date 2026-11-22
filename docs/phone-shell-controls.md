# Native scrcpy phone-shell controls

This branch adds an SDL3 phone-shell interface around the mirrored video. It reuses
the existing scrcpy Android control channel, extending it only for explicit
rotation presets and confirmed rotation/freeze-state acknowledgements.

## Controls

| Button | Action |
| --- | --- |
| PWR | Android POWER key press/release |
| VOL+ / VOL- | Android volume up/down keys |
| MUTE | Android VOLUME_MUTE (not microphone mute) |
| BACK | Android back key |
| HOME | Android home key |
| APPS | Android recent-apps/app-switch key |
| BELL | Expand Android notifications |
| SET | Expand Android Quick Settings |
| ROTATE | Advance one preset in a five-mode rotation loop |
| SHOT | Save a complete SDL window screenshot as BMP |
| STATE | Show Android rotation/frozen state diagnostics |
| LEDGER | Show the full ledger, including inactive/hidden controls |
| FULL | Toggle the scrcpy window's fullscreen mode |
| REC CLI | Non-interactive indicator; CLI recording only |

Mouse and touch events on the rails, navigation/footer, and header are intercepted
before Android screen injection; input inside the actual video is forwarded.
SHOT and STATE remain accessible at portrait widths of at least 350 logical px.
At smaller widths, optional header controls are omitted rather than overlapping.
Camera controls, torch, gamepad, and runtime recording are identified as hidden or
inactive in the ledger. No shake action is implemented.

## Rotation

ROTATE cycles in this order:

1. Auto (sensor): thaw rotation.
2. Portrait (0 degrees): freeze at rotation 0.
3. Landscape (90 degrees): freeze at rotation 1.
4. Reverse portrait (180 degrees): freeze at rotation 2.
5. Reverse landscape (270 degrees): freeze at rotation 3; next tap returns to Auto.

The Android server receives SC_CONTROL_MSG_TYPE_SET_ROTATION_MODE (mode 0 through 4).
It reports DEVICE_MSG_TYPE_ROTATION_STATE with the actual device display
rotation and whether that display is frozen. The DisplayMonitor also publishes
state changes as Android reports them.

The status is SYNCED only when Android confirms the expected rotation and
freeze/free state. PENDING means the command was requested but the device
has not confirmed it. This can occur if the phone, an app, or a virtual
display refuses forced orientation. The STATE panel shows the requested
preset, Android's actual rotation and frozen flag, and sync status.

Both the patched Windows client and patched Android server must be used
together: the additional protocol types are not available in stock scrcpy.

## Screenshots and recording

SHOT saves the current full SDL render target (phone display plus shell)
as scrcpy-shot-<epoch-seconds>-<SDL-ticks>.bmp. The destination is
SCRCPY_SCREENSHOT_DIR if set, otherwise the executable directory.
The destination must be writable. This is not video recording.
To record video use the normal scrcpy --record=FILE CLI option at startup.

## QA evidence (Admin PC, November 22, 2026)

- Compiled native client with SDL 3.4.20, GCC 16.2.0, Meson 1.12.1 and Ninja 1.13.2.
- Built the Android server with Java 17 / Android 35 build tools.
- Connected to Motorola Moto G 5G (2024), Android 15, over USB.
- All 14/14 Meson tests passed after the last compact-header adjustment.
- Confirmed LEDGER and STATE panels in a live scrcpy SDL window.
- Captured and inspected the SHOT BMP: 638 x 962, including video and shell.
- Auto and portrait requests achieved SYNCED during live tests.
- Some forced landscape requests remained PENDING because Android did not
  report a matching physical display rotation despite accepting the setting.
- Restored the phone's original portrait lock (rotation 0, sensor rotation off).

The functional UI currently uses simple SDL-drawn buttons; future cosmetics
(illustrated hardware icons and automatic shape-fit after external forced
window resizing) are not required for control-path correctness.
