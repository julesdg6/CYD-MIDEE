# GitHub Copilot Instructions for CYD-MIDI-Controller-EE

## Project Overview

CYD MIDI Controller - Enhanced Edition is an advanced touchscreen Bluetooth MIDI controller for the ESP32-2432S028R "Cheap Yellow Display" (CYD). This is a heavily modified fork focused on enhanced UI/UX with larger touch targets, improved layouts, and better touch accuracy for capacitive touchscreens.

**Key Features:**
- 15 interactive MIDI controller modes (keyboard, sequencer, generative music, etc.)
- Bluetooth MIDI wireless connectivity
- WiFi web server for file management and screenshots
- Multi-resolution support (480×320, 320×240)
- Persistent configuration via SD card
- Real-time MIDI output with configurable channels (1-16)

## Technology Stack

### Hardware Platform
- **Board**: ESP32-based CYD displays (ESP32-3248S035, ESP32-2432S028R, ESP32-2432S024)
- **Display**: ILI9488 (3.5") or ILI9341 (2.8"/2.4") TFT displays
- **Touch**: XPT2046 resistive touchscreen controller
- **Storage**: MicroSD card (optional) for screenshots and config
- **Connectivity**: Bluetooth LE MIDI, WiFi for web server

### Software Stack
- **Framework**: Arduino framework on ESP32
- **Build System**: PlatformIO (recommended) or Arduino IDE
- **Language**: C++ (Arduino dialect)
- **Key Libraries**:
  - `TFT_eSPI` v2.5.43+ - Display driver
  - `XPT2046_Touchscreen` - Touch input
  - ESP32 BLE libraries - Bluetooth MIDI
  - ESP32 SD/FS/WiFi libraries (built-in)

## Project Structure

```
CYD-MIDI-Controller-EE/
├── platformio.ini          # PlatformIO build configuration (3 board variants)
├── src/
│   ├── CYD-MIDI-Controller.ino  # Main file: setup, menu, mode switching
│   ├── common_definitions.h     # Shared types, colors, global state, threading
│   ├── ui_elements.h            # UI helpers (buttons, headers, touch)
│   ├── midi_utils.h/.cpp        # MIDI communication functions
│   ├── thread_manager.cpp       # Touch/MIDI threading system
│   ├── touch_calibration.h      # Touchscreen calibration
│   ├── web_server.h/.cpp        # WiFi web interface
│   └── *_mode.h/.cpp            # Individual mode implementations
├── BUILD.md                 # Build instructions and pin map reference
├── DEV_NOTES.md             # Step-by-step guide for creating new modes
├── THREADING.md             # Threading architecture documentation
└── assets/screenshots/      # Mode screenshots (BMP format)
```

## Architecture & Design Patterns

### Global State Management
- **GlobalState** (`globalState`): Shared BPM, MIDI channel, play state, BLE connection
- **MIDIClockSync** (`midiClock`): MIDI clock sync (legacy compatibility layer)
- **TouchState** (`touch`): Current touch position and press state

### Threading Architecture (Migration In Progress)
- **TouchThread**: Dedicated touch input on Core 0 (partially implemented)
- **MIDIThread**: Message queue for MIDI output on Core 1 (partially implemented)
- **Compatibility**: Legacy direct MIDI calls still in use during migration
- See `THREADING.md` for migration status and plans

### Mode System
Each mode is a self-contained module with:
- `initialize*Mode()` - Setup mode state
- `draw*Mode()` - Render UI (called once on entry)
- `handle*Mode()` - Process input in main loop
- Global state access via `globalState.bpm`, `globalState.currentMidiChannel`, etc.

### UI Conventions
- **Theme colors**: Defined in `common_definitions.h` (THEME_BG, THEME_PRIMARY, etc.)
- **Screen dimensions**: `SCREEN_WIDTH` and `SCREEN_HEIGHT` (auto-scaled per board)
- **Content area**: `CONTENT_TOP` (50px) below header
- **Touch targets**: Minimum 45px for buttons (60-80px recommended)
- **Headers**: Use `drawModuleHeader(title)` for consistent navigation

## Code Style & Conventions

### Naming
- **Variables**: camelCase (`keyboardOctave`, `lastBeatTime`)
- **Functions**: camelCase (`initializeKeyboardMode()`, `drawRoundButton()`)
- **Constants**: UPPER_SNAKE_CASE (`THEME_BG`, `NUM_KEYS`, `SCREEN_WIDTH`)
- **Classes**: PascalCase (`TouchThread`, `MIDIThread`, `GlobalState`)

### File Organization
- **Mode headers**: `*_mode.h` for small modes (header-only)
- **Mode implementations**: Split into `*_mode.h` + `*_mode.cpp` for complex modes (TB3PO, Grids, Raga, Euclidean, Morph)
- **Include order**: System libraries → Arduino libraries → Project headers

### Memory Management
- **Minimize heap allocation**: Use stack/static memory where possible
- **String usage**: Prefer Arduino `String` class for display text
- **Array sizes**: Use `#define` constants for fixed-size arrays

### Display Scaling
- **Always use** `SCREEN_WIDTH` and `SCREEN_HEIGHT` for layouts
- **Never hardcode** 320, 240, 480 dimensions
- **Test on multiple resolutions**: cyd35 (480×320), cyd28/cyd24 (320×240)

## Building & Testing

### Quick Build Commands
```bash
# Build for default board (cyd35 - 3.5" display)
pio run

# Build for specific board
pio run -e cyd35   # 3.5" 480×320
pio run -e cyd28   # 2.8" 320×240
pio run -e cyd24   # 2.4" 320×240

# Upload to device
pio run -t upload -e cyd35

# Monitor serial output
pio device monitor
```

### Testing Checklist
- **Touch calibration**: Run on first boot or after resolution change
- **Touch accuracy**: Verify button presses in all corners
- **Visual layout**: Check on target resolution (cyd35 vs cyd28/cyd24)
- **MIDI output**: Test with DAW or MIDI monitor
- **BLE connection**: Pair with "CYD MIDI" and verify connectivity
- **Mode switching**: Test navigation between modes
- **SD card**: Verify screenshot capture and config persistence

### Common Build Issues
- **Upload fails**: Lower speed to 115200 in `platformio.ini`
- **Display blank**: Check TFT pin configuration matches board variant
- **Touch not working**: Verify XPT2046 pins and calibration
- **BLE errors**: Ensure BLE is enabled in sketch (Settings menu)

## Hardware-Specific Considerations

### Board Variants
Select in `platformio.ini` (line 3):
- `default_envs = cyd35` - 480×320 ILI9488 (recommended)
- `default_envs = cyd28` - 320×240 ILI9341
- `default_envs = cyd24` - 320×240 ILI9341

### Pin Map (see BUILD.md for full reference)
- **Display SPI**: MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=27/21
- **Touch SPI**: MOSI=32, MISO=39, CLK=25, CS=33, IRQ=36
- **SD Card SPI**: MOSI=23, MISO=19, SCK=18, CS=5
- **Available GPIOs**: 22, 26, 35 (see BUILD.md for external hardware)

### Memory Constraints
- **Flash partition**: 3.1MB app size (huge_app.csv)
- **RAM**: ~320KB available (ESP32 dual-core)
- **Optimize for**: Code size over speed if near limits

## Adding New Features

### Adding a New Mode
Follow the step-by-step guide in `DEV_NOTES.md`:
1. Add mode to `AppMode` enum in `common_definitions.h`
2. Create `new_mode.h` (or split .h/.cpp for complex modes)
3. Include header in `CYD-MIDI-Controller.ino`
4. Add to initialization, loop, and `enterMode()` switch
5. Add entry to `apps[]` array
6. Add graphics case to `drawAppGraphics()`
7. Increment `numApps`

### Modifying Existing Modes
- **Keep existing behavior**: Minimize breaking changes
- **Test touch accuracy**: Verify button positions with calibration
- **Maintain theme colors**: Use defined THEME_* constants
- **Preserve MIDI functionality**: Test with actual MIDI hardware/software

### Threading Migration
Current modules use direct MIDI calls. Migrating to threaded architecture:
- Use `MIDIThread::sendNoteOn(note, velocity)` instead of `sendMIDI()`
- Reference `globalState.bpm` instead of module-local BPM
- See `THREADING.md` for migration status

## Common Development Tasks

### Debugging Touch Issues
```cpp
// Add debug output in ui_elements.h updateTouch()
Serial.printf("[TOUCH] Raw: (%d,%d) Mapped: (%d,%d)\n", rawX, rawY, touch.x, touch.y);
```

### Changing UI Colors
Edit `common_definitions.h`:
```cpp
#define THEME_PRIMARY    0x06FF  // 16-bit RGB565 color
```

### Adding Persistent Settings
Store in SD card `/wifi_config.txt` or similar:
```cpp
File file = SD.open("/my_config.txt", FILE_WRITE);
file.println(value);
file.close();
```

### Web Server Endpoints
Extend `web_server.cpp`:
- Root `/` - File browser
- `/screenshot` - Capture current screen
- `/upload` - File upload handler
- Add custom handlers via `server.on()`

## MIDI Implementation

### Sending MIDI Messages (Current)
```cpp
// Direct send (legacy - being replaced by threading)
sendMIDI(0x90 | (globalState.currentMidiChannel - 1), note, velocity);  // Note On
sendMIDI(0x80 | (globalState.currentMidiChannel - 1), note, 0);        // Note Off
sendMIDI(0xB0 | (globalState.currentMidiChannel - 1), cc, value);      // CC
```

### Sending MIDI Messages (Threaded - Future)
```cpp
// Queued send via thread (preferred when migration complete)
MIDIThread::sendNoteOn(note, velocity);
MIDIThread::sendNoteOff(note, 0);
MIDIThread::sendCC(cc, value);
```

### MIDI Channel Usage
- All modes respect `globalState.currentMidiChannel` (1-16)
- Configurable via Settings menu
- Convert to 0-indexed: `channel - 1` for MIDI messages

## Documentation

### Updating Documentation
- **README.md**: User-facing features and installation
- **BUILD.md**: Build instructions and hardware pin map
- **DEV_NOTES.md**: Developer guide for creating modes
- **THREADING.md**: Architecture migration progress
- **This file**: Copilot development instructions

### Screenshot Guidelines
- Format: BMP (SD card compatible)
- Location: `assets/screenshots/`
- Naming: `##_mode_name.bmp` (e.g., `03_keyboard.bmp`)
- Capture via Settings menu Screenshot Mode

## Best Practices

### Performance
- **Avoid blocking loops**: Use state machines for timing
- **Minimize redraws**: Only update changed screen areas when possible
- **Efficient touch handling**: Check `touch.justPressed` for single-shot actions
- **BPM timing**: Use `globalState.bpm` and `millis()` for tempo sync

### Code Quality
- **Comment complex logic**: Especially music theory (scales, euclidean rhythms)
- **Test on hardware**: Emulators don't capture touch accuracy issues
- **Validate inputs**: Constrain values before display or MIDI send
- **Handle SD card failures**: Check `sdCardAvailable` before file ops

### Git Workflow
- **Ignore build artifacts**: `.pio/`, `.vscode/`, `*.bin` already in `.gitignore`
- **Commit screenshots**: Optional - uncomment in `.gitignore` to exclude
- **Test before commit**: Verify compilation on at least one board variant

## Troubleshooting

### Touch Inaccuracy
1. Run calibration from Settings menu
2. Verify `SCREEN_WIDTH`/`SCREEN_HEIGHT` match board variant
3. Check touch swap/rotation settings in calibration
4. Ensure button sizes meet 45px minimum

### BLE Connection Issues
1. Toggle BLE off/on in Settings
2. Restart device
3. Check BLE paired devices and remove old pairings
4. Verify `bleEnabled` global variable

### SD Card Problems
1. Format as FAT32
2. Check SPI pin connections (HSPI, not VSPI)
3. Verify CS pin 5
4. Test with known-good card

### Web Server Not Accessible
1. Create `/wifi_config.txt` on SD card (SSID line 1, password line 2)
2. Check serial output for IP address
3. Fallback: Connect to AP "CYD-MIDI" (password: midi1234)
4. Access `http://[device-ip]`

## Resources

- **Original Project**: [NickCulbertson/CYD-MIDI-Controller](https://github.com/NickCulbertson/CYD-MIDI-Controller)
- **Hardware Info**: [ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- **TFT_eSPI Docs**: [Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- **PlatformIO**: [platformio.org](https://platformio.org/)

## Questions to Ask Before Making Changes

1. **Does this affect touch accuracy?** → Test on hardware with calibration
2. **Does this work on all resolutions?** → Use SCREEN_WIDTH/SCREEN_HEIGHT
3. **Is threading involved?** → Check THREADING.md for migration status
4. **Does this change MIDI behavior?** → Test with DAW/MIDI monitor
5. **Is SD card required?** → Handle gracefully if not available
6. **Does this add dependencies?** → Check library size impact
7. **Is this documented?** → Update relevant .md files

---

**When in doubt, refer to DEV_NOTES.md for mode creation or BUILD.md for hardware details.**
