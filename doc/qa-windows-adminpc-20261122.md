# Windows QA checkpoint — 2026-11-22

Repository: `nellyd95/scrcpy`  
Branch: `Contribute`  
Tested commit: `277a1534a46aae830c464694459fc2a42c795d37`

## Environment

- Host: Admin PC, Windows 10 x64
- Source: clean Git checkout on D:
- Build and test output: T:
- Toolchain: MSYS2 UCRT64, GCC 16.2, Meson 1.12.1, Ninja 1.13.2
- SDL3: 3.4.20
- FFmpeg libraries: libavcodec 63.24.100; libavformat 63.16.100; libavutil 61.19.100
- libusb: 1.0.30
- Android device visible via USB ADB: Moto G 5G (2024)

## Results

| Check | Result |
| --- | --- |
| Clean checkout on `Contribute` | PASS |
| Meson setup, `-Dcompile_server=false -Dportable=true -Dbuildtype=debug` | PASS |
| Native Windows client build (`scrcpy.exe`) | PASS |
| All Meson unit tests | PASS: 13/13 |
| Rotation sync unit test | PASS |
| `scrcpy.exe --version` | PASS |
| `scrcpy.exe --help` | PASS |
| `adb devices -l` via USB | PASS |
| Running scrcpy mirroring session with modified Android server | NOT TESTED |
| Physical-style UI buttons | NOT IMPLEMENTED |
| Five-mode Android rotation protocol and acknowledgement | NOT IMPLEMENTED |
| Full Windows wrapper UI and screenshots | NOT IMPLEMENTED |

## Reproduction

Use an MSYS2 UCRT64 environment. Configure against a checkout of the commit above:

```sh
meson setup build /path/to/scrcpy -Dcompile_server=false -Dportable=true -Dbuildtype=debug
meson compile -C build -j 4
meson test -C build --print-errorlogs
build/app/scrcpy.exe --version
```

The resulting client was built without a new Android server binary; therefore this is a native-client build/test and CLI smoke checkpoint, **not end-to-end validation of the complete wrapper**.

## Next QA gates

The UI, input controls, Android rotation protocol, and SDL synchronization must be implemented and compiled before performing live device checks. At that stage verify each button, five-mode rotation cycling, sensor-mode restoration, Android-to-SDL synchronization, disconnect/reconnect behavior, window sizing, screenshots, and video/audio regression behavior. Use USB ADB only unless separately authorized.
