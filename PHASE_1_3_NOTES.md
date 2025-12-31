# Phase 1.3 Implementation Notes - LVGL Hardware Initialization

**Date**: 2025-12-31  
**Status**: ✅ Implementation Complete - Awaiting Hardware Test  
**Issue**: Related to julesdg6/CYD-MIDEE#[phase-1-3]  
**Related PR**: julesdg6/CYD-MIDEE#[phase-1-3-pr]

## Overview

Phase 1.3 successfully integrates LVGL and esp32-smartdisplay library initialization alongside the existing TFT_eSPI display code. This creates a dual-display system during the migration period, allowing validation of LVGL functionality before fully removing legacy code.

## What Was Implemented

### 1. LVGL Test Mode (`lvgl_test_mode.h`)

A new interactive mode for validating LVGL hardware functionality:

**Features:**
- Full-screen LVGL UI (not using TFT_eSPI)
- Interactive button with press counter
- Touch coordinate display (live updates)
- Back button navigation
- Theme-matched colors (THEME_BG, THEME_PRIMARY, etc.)

**Purpose:**
- Validate LVGL display rendering
- Test LVGL touch input driver
- Confirm esp32-smartdisplay board detection
- Verify event system works correctly

### 2. Hardware Initialization Changes

**Setup Order (critical):**
```cpp
1. TFT_eSPI initialization (legacy, for backward compatibility)
2. XPT2046 touch initialization (legacy)
3. SD card and calibration
4. BLE MIDI setup
5. Thread managers (TouchThread, MIDIThread)
6. LVGL/smartdisplay initialization ← NEW (Phase 1.3)
7. Menu drawing (TFT_eSPI)
```

**LVGL Init Code:**
```cpp
smartdisplay_init();  // Auto-detects board, sets up display + touch
lvglInitialized = true;
lv_last_tick = millis();

// Set rotation to landscape (match TFT_eSPI)
auto display = lv_display_get_default();
lv_display_set_rotation(display, LV_DISPLAY_ROTATION_90);
```

### 3. Main Loop Integration

**LVGL Ticker:**
```cpp
void loop() {
  // LVGL ticker (must be called every loop iteration)
  if (lvglInitialized) {
    unsigned long now = millis();
    lv_tick_inc(now - lv_last_tick);  // Update LVGL internal timer
    lv_last_tick = now;
    lv_timer_handler();  // Process LVGL tasks (animations, inputs, etc.)
  }
  
  // Rest of main loop...
}
```

**Why This Matters:**
- `lv_tick_inc()`: Updates LVGL's internal millisecond counter
- `lv_timer_handler()`: Processes pending LVGL tasks
- Must be called frequently (every 20ms or less) for smooth UI

### 4. Menu System Updates

**App Grid:**
- Expanded from 15 to 16 apps
- LVGL test mode appears as 16th icon (bottom row)
- Icon: Green checkmark (validation symbol)

**Screenshot System:**
- Now captures 19 screenshots (was 18):
  - 3 system screens (menu, settings, BLE)
  - 16 mode screens (15 existing + 1 new LVGL test)

## Technical Specifications

### Display Configuration

**Resolution Support:**
- cyd35: 480×320 (ILI9488/ST7796)
- cyd28: 320×240 (ILI9341)
- cyd24: 320×240 (ILI9341)

**Rotation:**
- TFT_eSPI: rotation = 1 (landscape)
- LVGL: LV_DISPLAY_ROTATION_90 (landscape)
- Both aligned to same coordinate system

### Memory Configuration

**From `lv_conf.h`:**
- Color depth: 16-bit (RGB565)
- Memory pool: 64KB
- Refresh rate: 33ms (30 FPS)
- Touch read period: 30ms
- Performance monitor: Enabled (bottom-right corner)
- Memory monitor: Enabled (top-right corner)

**Build Impact:**
- Expected binary size increase: ~200KB (LVGL library)
- Current build: 1.73MB / 3.14MB (54.9%)
- Post-LVGL estimate: ~1.93MB / 3.14MB (~61%)
- Still well under 3.1MB flash limit ✅

### Touch Input

**Dual System (Phase 1.3):**
- **TFT_eSPI modes**: Use XPT2046_Touchscreen directly
- **LVGL test mode**: Uses esp32-smartdisplay touch driver

**Future (Phase 2+):**
- All modes will use LVGL touch input
- XPT2046_Touchscreen library removed
- Single unified touch handling

## Code Architecture

### Event-Driven UI Pattern

The LVGL test mode demonstrates the future architecture:

```cpp
// Button creation
test_button = lv_btn_create(test_screen);
lv_obj_add_event_cb(test_button, button_event_handler, LV_EVENT_CLICKED, NULL);

// Event handler (callback)
static void button_event_handler(lv_event_t* e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    // Handle button press
    button_press_count++;
    lv_label_set_text(test_button_label, text.c_str());
  }
}

// Main loop - NO manual touch handling!
void handleLVGLTestMode() {
  // Only update non-interactive elements (coordinate display)
  // LVGL handles all button presses automatically
}
```

**Key Principles:**
1. ✅ UI components register event callbacks
2. ✅ LVGL calls callbacks when events occur
3. ✅ Main loop only updates non-interactive content
4. ❌ NO manual touch coordinate checking
5. ❌ NO `if (touch.justPressed && isButtonPressed(...))` patterns

### Lifecycle Management

**Mode Entry:**
```cpp
case LVGL_TEST:
  if (lvglInitialized) {
    initializeLVGLTestMode();  // Create LVGL screen and widgets
    drawLVGLTestMode();        // No-op (LVGL handles drawing)
  } else {
    Serial.println("ERROR: LVGL not initialized");
    exitToMenu();
  }
  break;
```

**Mode Exit:**
```cpp
void exitToMenu() {
  if (currentMode == LVGL_TEST) {
    cleanupLVGLTestMode();  // Delete LVGL objects
  }
  // ... rest of exit logic
}
```

**Why Cleanup Matters:**
- LVGL objects consume memory
- Must be deleted when mode exits
- Prevents memory leaks
- `lv_obj_del(screen)` deletes object and all children

## Testing Checklist

### Build Verification ⏳
- [ ] Compiles without errors on cyd28
- [ ] Compiles without errors on cyd35
- [ ] Compiles without errors on cyd24
- [ ] Binary size under 3.1MB
- [ ] No memory allocation warnings

### Hardware Testing (cyd28) ⏳
- [ ] Device boots successfully
- [ ] TFT_eSPI modes still work (legacy)
- [ ] LVGL test mode appears in menu
- [ ] LVGL test mode loads without errors
- [ ] Display renders LVGL UI correctly
- [ ] Touch button responds to presses
- [ ] Touch coordinates displayed accurately
- [ ] Back button returns to menu
- [ ] Memory usage stable (no leaks)

### Hardware Testing (cyd35) ⏳
- [ ] Same checklist as cyd28
- [ ] Verify 480×320 resolution renders correctly
- [ ] Touch scaling matches display

### Hardware Testing (cyd24) ⏳
- [ ] Same checklist as cyd28
- [ ] Verify 320×240 resolution matches cyd28

### Touch Accuracy Validation ⏳
- [ ] LVGL touch coordinates match TFT_eSPI coordinates
- [ ] Buttons in all corners respond correctly
- [ ] No offset/skew in touch mapping
- [ ] Rotation correct (landscape orientation)

## Known Issues / Limitations

### Current Limitations

1. **Dual Display System**
   - Both TFT_eSPI and LVGL initialized
   - Memory overhead during migration
   - Will be resolved in Phase 2 (remove TFT_eSPI)

2. **LVGL Performance Monitors**
   - FPS/memory displays enabled for debugging
   - May impact performance slightly
   - Should be disabled in production (Phase 10)

3. **Touch Driver Duplication**
   - XPT2046 initialized twice (once for TFT_eSPI, once for LVGL)
   - Could cause conflicts if not managed carefully
   - Ordering in setup() critical

### Potential Issues (Not Yet Tested)

1. **SPI Bus Contention**
   - Display, touch, and SD card share SPI buses
   - esp32-smartdisplay should handle this, but test needed

2. **Memory Pressure**
   - 64KB LVGL pool + existing allocations
   - Watch for heap fragmentation
   - May need to increase LVGL pool or use PSRAM

3. **Frame Rate**
   - LVGL animations may impact MIDI timing
   - Test with sequencer modes running
   - May need to adjust lv_timer_handler() frequency

## Next Phase Preview (Phase 2)

### What Comes Next

**Phase 2.1: UI Component Library**
- Create LVGL wrapper classes (UIButton, UISlider, etc.)
- Match existing event-driven architecture
- Integrate with UIManager

**Phase 2.2: Common UI Patterns**
- Migrate `drawModuleHeader()` to LVGL
- Create standard back button component
- Create BLE/BPM status indicators

**Example Future Component:**
```cpp
// Future API (Phase 2)
UIButton* btn = new UIButton(x, y, w, h, "Press Me", THEME_PRIMARY);
btn->onPress([]() {
  handleButtonPress();
});
UIManager::registerButton(btn);
```

## Success Criteria

### Phase 1.3 Complete When:
- ✅ LVGL initializes without errors
- ✅ LVGL test mode renders correctly
- ✅ Touch input works in LVGL mode
- ✅ Legacy TFT_eSPI modes still work
- ✅ No memory leaks or crashes
- ✅ Build size under 3.1MB
- ✅ All board variants tested (cyd28, cyd35, cyd24)

### Validation Gates:
1. **Compilation** ← Current stage (pending platform install)
2. **Hardware Boot** ← Next stage
3. **LVGL Display** ← Next stage
4. **Touch Input** ← Next stage
5. **Multi-Board** ← Final stage

## Files Modified

### Core Changes
- `src/common_definitions.h` - Added LVGL_TEST enum
- `src/CYD-MIDI-Controller.ino` - LVGL init, ticker, mode handling
- `src/lvgl_test_mode.h` - New LVGL test mode (NEW FILE)

### Configuration (Already Done in Phase 1.2)
- `platformio.ini` - LVGL/smartdisplay dependencies
- `src/lv_conf.h` - LVGL configuration

### Documentation (This File)
- `PHASE_1_3_NOTES.md` - Implementation notes (NEW FILE)

## References

### Documentation
- [LVGL Documentation](https://docs.lvgl.io/master/)
- [esp32-smartdisplay GitHub](https://github.com/rzeldent/esp32-smartdisplay)
- [LVGL_MIGRATION_PLAN.md](LVGL_MIGRATION_PLAN.md)

### Related Issues/PRs
- Phase 1.2: Build System Migration (PR #79, completed)
- Phase 1.3: Hardware Init Refactor (current)
- Phase 2.1: UI Component Library (upcoming)

## Troubleshooting Guide

### Build Errors

**Error: LVGL headers not found**
- Solution: Ensure platformio.ini has `lvgl/lvgl @ ^9.1.0`
- Run: `platformio lib install`

**Error: esp32_smartdisplay not found**
- Solution: Ensure platformio.ini has `rzeldent/esp32_smartdisplay @ ^3.0.8`
- Run: `platformio lib install`

**Error: Multiple definition of lv_*****
- Solution: Check that LV_CONF_INCLUDE_SIMPLE=1 in build_flags
- Ensure lv_conf.h is in src/ directory

### Runtime Errors

**Display blank in LVGL test mode**
- Check: Serial output for LVGL init errors
- Check: `lvglInitialized` flag is true
- Check: Display rotation set correctly

**Touch not working in LVGL mode**
- Check: esp32-smartdisplay auto-detected board correctly
- Check: Touch coordinates displayed (should update on press)
- Compare: TFT_eSPI touch coords vs LVGL coords

**Crash on mode entry**
- Check: LVGL objects created before screen loaded
- Check: Event callbacks use static functions
- Check: No null pointer dereferences in event handlers

### Memory Issues

**Out of memory errors**
- Check: LVGL memory pool size (lv_conf.h LV_MEM_SIZE)
- Monitor: Serial output for heap fragmentation warnings
- Consider: Increasing LV_MEM_SIZE to 96KB or 128KB

**Memory leaks**
- Check: `cleanupLVGLTestMode()` called on mode exit
- Check: All `lv_obj_create()` calls have matching `lv_obj_del()`
- Monitor: Heap free memory over time

## Author Notes

This implementation follows the LVGL migration plan (Option A) from LVGL_MIGRATION_PLAN.md. Phase 1.3 is the first step where LVGL actually renders to the display.

Key design decisions:
1. **Dual initialization** - Keep TFT_eSPI working during migration
2. **Test mode first** - Validate LVGL before migrating real modes
3. **Event-driven** - LVGL test mode demonstrates future architecture
4. **Minimal changes** - Only add what's needed, don't remove legacy code yet

The LVGL test mode serves as:
- Hardware validation tool
- Touch accuracy tester
- Reference implementation for Phase 2
- Developer learning tool

---

**Next Step**: Hardware test on actual CYD device to validate implementation.
