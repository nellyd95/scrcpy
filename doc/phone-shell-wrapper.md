# Windows phone-shell wrapper and SDL control integration

## Design target

The wrapper stays inside this scrcpy repository and reuses the existing ADB connection, video decoder, SDL renderer, input manager, and Android control channel. It must not create a second mirroring pipeline.

The supplied UI concept is the visual reference: native Windows title and toolbar; centered phone viewport with a thin device frame and compact physical controls attached to the frame; a button ledger; rotation diagnostics; five rotation modes; and a connection/status footer. The desktop chrome should remain compact and the phone controls should be visually flush with the rendered device rather than floating as bulky external widgets.

## Rotation model

The client rotation state model in app/src/rotation_sync.c and app/src/rotation_sync.h provides the five-mode cycle. Android rotation values 0, 1, 2, and 3 correspond to 0, 90, 180, and 270 degrees. Auto means no forced rotation.

The Android device is authoritative. A forced-mode tap is a request, not an acknowledgement. SDL orientation must not be committed to a requested forced value until Android reports that value. In Auto mode, every confirmed Android rotation updates the SDL transform. This prevents a client window transform from being mistaken for an Android rotation.

## Required protocol work before end-to-end sync

The current device-to-client control channel carries clipboard, clipboard acknowledgements, and UHID output only. It does not report Android display rotation. The existing ROTATE_DEVICE command toggles portrait/landscape and cannot represent all four forced orientations or Auto.

The next integration stage must extend both protocol ends in lockstep:
1. Add a set-rotation-mode command with an explicit mode value.
2. Apply forced modes using the existing Android WindowManager wrapper; Auto must thaw rotation.
3. Use the existing DisplayMonitor to report confirmed display rotation and whether rotation is frozen.
4. Add a validated device message for rotation state. Do not infer orientation solely from video dimensions because reverse portrait and reverse landscape share dimensions with their forward counterparts.
5. Marshal rotation messages to the SDL main thread, update the rotation state, and call sc_screen_set_orientation() only after device confirmation.
6. Ignore duplicate reports and distinguish client-originated confirmations from external sensor/user rotation changes.

## Button ledger contract

The ledger must define every mapped action and separate visible/on-frame actions, visible toolbar actions, hidden/inactive actions (documented but not drawn or clickable), and unsupported actions (shown as unavailable, not as working). Physical controls must call the same native input/control functions as keyboard shortcuts; they must not synthesize mouse clicks against the phone video.

## Verification

The pure rotation state model has a dedicated unit-test target. End-to-end verification still requires a Windows build and a real Android device. Test sensor rotation, all four forced modes, Auto restoration, disconnect/reconnect, and duplicate state reports.
