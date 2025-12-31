# Phase 1.3 Hardware Testing Checklist

**Purpose**: Validate LVGL hardware initialization on actual CYD devices  
**Prerequisites**: Phase 1.3 code complete, firmware compiled  
**Target Boards**: cyd28 (2.8"), cyd35 (3.5"), cyd24 (2.4")

---

## Pre-Test Setup

### Build the Firmware

```bash
# Install PlatformIO if not already installed
pip install platformio

# Build for target board
cd /path/to/CYD-MIDEE
platformio run -e cyd28    # or cyd35, cyd24

# Upload to device
platformio run -t upload -e cyd28

# Monitor serial output
platformio device monitor -b 115200
```

### Expected Build Output

- ✅ Compilation completes without errors
- ✅ Flash usage: ~1.93MB / 3.14MB (~61%)
- ✅ RAM usage: ~100KB / 320KB (~30%)
- ✅ Warning about TOUCH_CS (expected until Phase 10)

---

## Test 1: Device Boot

### Steps
1. Power on device
2. Watch serial monitor during boot

### Expected Results
- ✅ Serial output shows "CYD MIDI Controller Starting..."
- ✅ TFT_eSPI initialization messages
- ✅ SD card detected (if present)
- ✅ Touch calibration loaded
- ✅ BLE MIDI initialization
- ✅ Thread managers started
- ✅ **NEW**: "=== LVGL Initialization (Phase 1.3) ==="
- ✅ **NEW**: "LVGL display initialized and rotated to landscape"
- ✅ **NEW**: "LVGL initialization complete!"
- ✅ Main menu displays

### Pass Criteria
All initialization messages appear without errors

### Fail Actions
- Check serial output for error messages
- Verify board variant matches platformio.ini
- Check power supply (5V, 1A minimum)

---

## Test 2: Legacy Modes Still Work

### Steps
1. From main menu, tap any of the first 15 mode icons
2. Verify mode loads correctly
3. Test touch interaction
4. Use back button to return to menu
5. Repeat for 2-3 different modes

### Expected Results
- ✅ All legacy modes still functional
- ✅ Touch response normal
- ✅ Display rendering correct
- ✅ MIDI output works (if BLE connected)

### Pass Criteria
No regressions in existing functionality

### Fail Actions
- If specific mode broken: Check mode-specific code
- If all modes broken: LVGL may be interfering with TFT_eSPI
- Check SPI bus conflicts

---

## Test 3: LVGL Test Mode Entry

### Steps
1. Return to main menu
2. Locate LVGL test icon:
   - **Location**: Bottom row, first position (16th icon)
   - **Appearance**: Green checkmark symbol
   - **Label**: "LVGL"
3. Tap the icon

### Expected Results
- ✅ Screen changes to LVGL test UI
- ✅ Display shows:
   - "LVGL HARDWARE TEST" header (cyan/blue)
   - "Click Me!" button (centered)
   - "Touch: (0, 0)" label (bottom)
   - Info text about test purpose
   - "BACK" button (top-left)
- ✅ Serial output: "LVGL Test Mode initialized successfully"

### Pass Criteria
LVGL UI renders without graphical glitches

### Fail Actions
- If screen blank: Check serial for LVGL errors
- If garbled: Check display rotation settings
- If crash: Check memory allocation

---

## Test 4: Touch Input - Button

### Steps
1. While in LVGL test mode
2. Tap the center "Click Me!" button
3. Observe button press counter
4. Tap multiple times

### Expected Results
- ✅ Button responds to taps
- ✅ Counter increments each press
- ✅ Button label updates: "Pressed 1 times", "Pressed 2 times", etc.
- ✅ Serial output: "LVGL Test Button clicked! Count: X"
- ✅ Visual feedback on button press (if theme supports it)

### Pass Criteria
- Button responds reliably to touch
- Counter increments correctly
- No missed taps or false triggers

### Fail Actions
- If no response: Check LVGL touch driver initialization
- If offset: Check touch calibration/mapping
- If unreliable: Adjust touch sensitivity or debouncing

---

## Test 5: Touch Coordinates

### Steps
1. While in LVGL test mode
2. Touch various screen locations:
   - Top-left corner
   - Top-right corner
   - Bottom-left corner
   - Bottom-right corner
   - Center
3. Watch "Touch: (x, y)" label update

### Expected Results
- ✅ Coordinates display updates in real-time
- ✅ Touch coordinates match touch location:
   - cyd28/cyd24: 0-319 (X), 0-239 (Y)
   - cyd35: 0-479 (X), 0-319 (Y)
- ✅ No offset or skew
- ✅ Landscape orientation correct

### Pass Criteria
- Touch mapping accurate across full screen
- No dead zones
- Coordinates update smoothly

### Fail Actions
- If offset: Recalibrate touch or check rotation settings
- If inverted: Check XY swap in LVGL configuration
- If wrong range: Verify board variant detection

---

## Test 6: Back Button Navigation

### Steps
1. While in LVGL test mode
2. Tap the "BACK" button (top-left)

### Expected Results
- ✅ Returns to main menu
- ✅ Serial output: "LVGL Test Mode: Back button pressed, exiting to menu"
- ✅ Serial output: "LVGL Test Mode cleaned up"
- ✅ No visual artifacts
- ✅ Menu renders correctly

### Pass Criteria
- Clean exit from LVGL mode
- Memory properly released
- Can re-enter mode multiple times without issues

### Fail Actions
- If crash on exit: Check cleanup code in cleanupLVGLTestMode()
- If memory leak: Monitor heap usage over multiple entries/exits
- If artifacts: May need to clear screen before menu redraw

---

## Test 7: LVGL Performance

### Steps
1. Enter LVGL test mode
2. Observe FPS counter (bottom-right corner)
3. Observe memory usage (top-right corner)
4. Interact with button repeatedly

### Expected Results
- ✅ FPS: 25-30 FPS (target: 30 FPS)
- ✅ Memory usage stable
- ✅ No stuttering or lag
- ✅ Smooth animations (if any)

### Pass Criteria
- Frame rate acceptable for UI interaction
- Memory usage doesn't increase over time
- No performance degradation

### Fail Actions
- If low FPS: May need to optimize lv_timer_handler frequency
- If memory increasing: Check for memory leaks
- If stuttering: Check for blocking operations in main loop

---

## Test 8: Long-Running Stability

### Steps
1. Enter and exit LVGL test mode 10 times
2. Press test button 50+ times
3. Leave in LVGL test mode for 5 minutes

### Expected Results
- ✅ No crashes
- ✅ Memory usage stable (check serial monitor)
- ✅ Touch remains responsive
- ✅ No visual corruption

### Pass Criteria
- Stable operation over extended period
- No memory leaks detected
- Consistent performance

### Fail Actions
- If crash: Check for buffer overflows or memory issues
- If slowdown: Memory fragmentation possible
- If corruption: Display driver issue

---

## Test 9: Multi-Board Validation

### Steps
1. Repeat Tests 1-8 on all board variants:
   - cyd28 (2.8" 320×240 ILI9341)
   - cyd35 (3.5" 480×320 ILI9488)
   - cyd24 (2.4" 320×240 ILI9341)

### Expected Results
- ✅ All tests pass on all boards
- ✅ Touch accuracy good on all boards
- ✅ Display scaling correct for each resolution
- ✅ Auto-detection works (esp32-smartdisplay)

### Pass Criteria
- Consistent behavior across all board variants
- No board-specific issues

### Fail Actions
- If board-specific issue: Check board definition in platformio.ini
- If scaling wrong: Verify SCREEN_WIDTH/SCREEN_HEIGHT
- If touch off: Re-calibrate for that board

---

## Test 10: Regression Check

### Steps
1. Test all 15 legacy modes briefly:
   - Keyboard - play notes
   - Sequencer - start/stop playback
   - XY Pad - move touch around
   - TB3PO - edit pattern
   - Etc.

### Expected Results
- ✅ All legacy modes work as before
- ✅ No MIDI issues
- ✅ No touch issues
- ✅ No display issues

### Pass Criteria
- Zero regressions in existing functionality
- LVGL integration doesn't break anything

### Fail Actions
- If regression found: Check for SPI conflicts
- If MIDI timing off: LVGL ticker may be affecting timing
- If display glitches: TFT_eSPI/LVGL conflict

---

## Test Results Template

Copy this to document your test results:

```markdown
## Phase 1.3 Hardware Test Results

**Date**: YYYY-MM-DD
**Tester**: [Your Name]
**Board**: cyd28 / cyd35 / cyd24
**Firmware**: [Git commit hash]

### Test Results

| Test # | Test Name | Pass/Fail | Notes |
|--------|-----------|-----------|-------|
| 1 | Device Boot | ☐ Pass ☐ Fail | |
| 2 | Legacy Modes | ☐ Pass ☐ Fail | |
| 3 | LVGL Entry | ☐ Pass ☐ Fail | |
| 4 | Button Touch | ☐ Pass ☐ Fail | |
| 5 | Touch Coords | ☐ Pass ☐ Fail | |
| 6 | Back Button | ☐ Pass ☐ Fail | |
| 7 | Performance | ☐ Pass ☐ Fail | FPS: ___ |
| 8 | Stability | ☐ Pass ☐ Fail | |
| 9 | Multi-Board | ☐ Pass ☐ Fail | |
| 10 | Regression | ☐ Pass ☐ Fail | |

### Serial Output (First Boot)
```
[Paste relevant serial output here]
```

### Observations
[Any additional notes, issues, or observations]

### Screenshots
[If possible, attach photos of display during test]

### Conclusion
☐ Phase 1.3 PASSED - Ready for Phase 2
☐ Phase 1.3 FAILED - Issues need resolution

### Issues Found
1. [Issue description]
2. [Issue description]

### Recommendations
[Any suggestions for improvements or fixes]
```

---

## Success Criteria

Phase 1.3 is considered **PASSED** when:

- ✅ All 10 tests pass on at least one board (cyd28)
- ✅ Tests 1-8 pass on all three boards (multi-board)
- ✅ No critical bugs found
- ✅ No regressions in existing functionality
- ✅ LVGL performance acceptable (25+ FPS)
- ✅ Memory usage stable

Phase 1.3 is considered **FAILED** if:

- ❌ LVGL fails to initialize
- ❌ Display corruption in LVGL mode
- ❌ Touch completely non-functional
- ❌ Crashes or freezes
- ❌ Critical regressions in legacy modes

---

## Next Steps After Testing

### If PASSED ✅
1. Document test results
2. Update PHASE_1_3_NOTES.md with findings
3. Merge PR to main branch
4. Create Phase 2.1 issue (UI Component Library)
5. Begin planning first mode migration

### If FAILED ❌
1. Document all issues found
2. Create GitHub issues for each problem
3. Fix critical issues
4. Re-test
5. Repeat until passed

---

## Contact

If you encounter issues during testing:
- Check PHASE_1_3_NOTES.md troubleshooting section
- Review serial output for error messages
- Check LVGL documentation: https://docs.lvgl.io
- Review esp32-smartdisplay examples: https://github.com/rzeldent/esp32-smartdisplay

---

*Phase 1.3 Hardware Testing Checklist v1.0*  
*Last updated: 2025-12-31*
