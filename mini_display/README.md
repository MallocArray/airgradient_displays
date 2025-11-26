# AirGradient DIY Display — CYD Hardware Port

This repository contains a port of the AirGradient DIY Display firmware to the CYD hardware platform. It adapts the original AirGradient display code to run on the CYD board and provides PlatformIO project configuration so you can build and upload the firmware using Visual Studio Code.

## What this is

- A firmware build for the AirGradient DIY Display targeting the CYD hardware variant.
- PlatformIO project files and source sketches are included so you can build and flash from VS Code.

This project is intended for hobbyists and makers who want to run the AirGradient display code on CYD-compatible hardware. It assumes you have a CYD board and the required sensors and wiring for the AirGradient display.

## Requirements

- Visual Studio Code (latest stable)
- PlatformIO extension for VS Code
- USB cable compatible with your CYD board
- Windows PowerShell (instructions below use PowerShell)
- Any hardware wiring described in the original AirGradient DIY Display documentation (sensors, display, wiring)

## Quick start — Build and upload using VS Code + PlatformIO

1. Open this folder in Visual Studio Code. Use `File -> Open Folder...` and select the `mini_display` folder.
2. Install the PlatformIO extension if you haven't already. In VS Code, open the Extensions view and search for "PlatformIO IDE" and install it.
3. Confirm PlatformIO recognizes the project. The `platformio.ini` in this folder contains the build configuration for the CYD target. You should see PlatformIO status in the bottom bar of VS Code.
4. Build the firmware:

```powershell
# From the terminal inside VS Code (PowerShell) run:
platformio run
```

5. Upload the firmware to your CYD board. First connect your board to USB and make sure the correct serial/USB port is available. Then run:

```powershell
# Builds and uploads using the environment configured in platformio.ini
platformio run --target upload
```

Alternatively, use the PlatformIO GUI: open the PlatformIO sidebar, choose "Project Tasks > [environment name] > Build" then "Upload".

## Notes about serial port and board selections

- If your board requires a specific upload protocol or serial port definition, edit `platformio.ini` and set `upload_port` or the appropriate `board`/`platform` entries. See PlatformIO documentation for details.
- On Windows, the serial port will be like `COM3`, `COM4`, etc. You can get the port in PowerShell with:

```powershell
# List serial devices (PowerShell)
Get-WmiObject Win32_SerialPort | Select-Object DeviceID,Caption
```

## Troubleshooting

- Build errors: ensure PlatformIO dependencies are installed (PlatformIO will attempt to download platforms and libraries automatically). If the build fails due to missing libraries, open the `platformio.ini` and check the `lib_deps` section.
- Upload errors: confirm the board is in the correct bootloader/upload mode (some boards require holding a button or resetting while the serial port is active). Also ensure no other program (like a serial monitor) is keeping the port open.
- If PlatformIO can't detect the board: verify the `board` field in `platformio.ini` matches a supported board, or set `upload_port` explicitly.

## Contributing and upstream

This repository is a hardware-specific port. For changes to the core AirGradient display features, consider upstreaming bug fixes or enhancements back to the original project repository. When contributing, include details about the CYD hardware revision and wiring used for testing.

## License and attribution

This project adapts AirGradient DIY Display code. Respect the original project's license and attribution. If you publish derived works, keep license and attribution information intact per the original project's requirements.
