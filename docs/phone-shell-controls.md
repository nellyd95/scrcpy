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

## Implemented rotation policy and state flow

- Forced modes temporarily set Android's fixed-to-user-rotation policy
  to `enabled`, then lock the requested rotation (0, 1, 2 or 3).
- Auto sets fixed-to-user-rotation to `disabled`, releases the rotation
  lock, and enables the `accelerometer_rotation=1` system setting.
- The server snapshots the previous fixed policy, user rotation,
  frozen/free state and accelerometer setting before the first request.
- Normal shutdown restores those original device settings.
- For ROMs without the fixed-policy getter on the `IWindowManager` binder
  proxy, the server uses Android's own `cmd window` interface to query
  and change that policy.
- The client adopts Android's authoritative orientation on startup and
  after external mode changes, while preserving pending local requests
  until a matching device acknowledgement arrives.

## Verified Admin PC results — November 22, 2026

- Motorola Moto G 5G (2024), Android 15, connected by USB ADB.
- Portrait: display rotation 0, user rotation `lock 0`, UI `SYNCED`.
- Landscape: rotation 1, user rotation `lock 1`, UI `SYNCED`.
- Reverse Portrait: rotation 2, user rotation `lock 2`, UI `SYNCED`.
- Reverse Landscape: rotation 3, user rotation `lock 3`, UI `SYNCED`.
- Auto (Sensor): user rotation `free`, fixed policy `disabled`,
  accelerometer setting 1, UI `SYNCED`.
- Closing the session restored original `lock 0`, `default` and
  `accelerometer_rotation=0`.
- Restarting while locked Portrait produced `Portrait — SYNCED`.
- Native client: all 14 Meson tests passed after startup-sync changes.
- Windows client and Android server rebuilt successfully.

See [rotation QA details](rotation-qa-20261122.md). Movement of the
physical handset under Auto was not independently tested remotely.
