# ESP32-SmartDisplay Library Integration Research

**Date**: 2025-12-31  
**Issue**: julesdg6/CYD-MIDEE#[refactor-use-esp32-smartdisplay]  
**Researcher**: GitHub Copilot

## Executive Summary

The `rzeldent/esp32-smartdisplay` library **cannot be integrated** into this project without a complete migration from TFT_eSPI to LVGL. The library is fundamentally architected around LVGL and provides no standalone hardware abstraction layer.

## Library Analysis

### What the Library Provides

1. **Hardware Abstraction** (LVGL-based only)
   - Automatic board detection for 18+ Sunton CYD variants
   - Display initialization (ILI9341, ILI9488, ST7796, ST7789, etc.)
   - Touch initialization (XPT2046 resistive, GT911/CST816S capacitive)
   - Correct pin configuration per board
   - Automatic rotation/mirror settings

2. **LVGL Integration** (Required, not optional)
   - LVGL display driver setup
   - LVGL touch input driver setup
   - Tick handler integration
   - Task handler integration

### What the Library DOES NOT Provide

- ❌ Standalone TFT_eSPI initialization
- ❌ Hardware abstraction without LVGL
- ❌ API to extract configured TFT/Touch objects
- ❌ Compatibility layer for existing TFT_eSPI code

### API Signature

```cpp
#include <esp32_smartdisplay.h>

void setup() {
  smartdisplay_init();  // Initializes LVGL display + touch

  // Must use LVGL for drawing from here on
  auto display = lv_display_get_default();
  lv_display_set_rotation(display, LV_DISPLAY_ROTATION_90);
}

void loop() {
  auto now = millis();
  lv_tick_inc(now - lv_last_tick);  // Required: LVGL ticker
  lv_last_tick = now;
  lv_task_handler();  // Required: LVGL event processing
}
```

**No way to use TFT_eSPI after calling `smartdisplay_init()`.**

## Current CYD-MIDEE Implementation

- **Display Library**: TFT_eSPI (Bodmer/TFT_eSPI @ ^2.5.43)
- **Touch Library**: XPT2046_Touchscreen (paulstoffregen/XPT2046_Touchscreen)
- **Drawing Approach**: Immediate mode rendering
- **UI Implementation**: Custom, ~5000 LOC across 15 MIDI modes
- **Dependencies**: No LVGL

## Integration Options

### Option A: Full LVGL Migration ⚠️ HIGH EFFORT

**Scope:**
- Replace all TFT_eSPI drawing with LVGL widgets
- Rewrite all 15 MIDI modes
- Learn LVGL API and design patterns
- Potentially use SquareLine Studio for UI design

**Estimated Effort:**
- Research/learning: 1-2 weeks
- Mode rewrites: 2-3 days per mode × 15 = 6-8 weeks
- Testing/debugging: 1-2 weeks
- **Total: 8-12 weeks**

**Benefits:**
- ✅ Professional UI framework
- ✅ Automatic hardware abstraction
- ✅ Better widget system (buttons, sliders, etc.)
- ✅ Themes and styling support
- ✅ Better long-term maintainability

**Risks:**
- ⚠️ Binary size increase (~200KB+ for LVGL)
- ⚠️ May exceed 3.1MB flash partition
- ⚠️ Performance overhead (LVGL rendering)
- ⚠️ High risk of regression bugs
- ⚠️ Months of development time

**Recommendation**: ❌ **NOT RECOMMENDED** unless:
1. User explicitly requests full UI redesign
2. User allocates 3+ months for development
3. User accepts potential binary size issues

---

### Option B: Use Board Definitions Only ✅ LOW EFFORT

**Scope:**
- Keep TFT_eSPI and XPT2046_Touchscreen
- **Fix driver mismatch for cyd35** (ILI9488 → ST7796)
- Update platformio.ini to reference Sunton board variants (optional)
- Verify rotation/mirror settings match board specs

**Estimated Effort:**
- Driver fix and testing: 1-2 hours
- platformio.ini board reference updates (optional): 1 hour
- Testing on all 3 variants: 2 hours
- **Total: 3-5 hours**

**Benefits:**
- ✅ Fixes potential cyd35 display issues
- ✅ Correct hardware configuration guaranteed
- ✅ Low risk of bugs
- ✅ Keeps existing codebase intact
- ✅ Documents correct board settings

**Implementation:**
1. **Fix cyd35 driver** (platformio.ini):
   ```ini
   [env:cyd35]
   build_flags =
     # Change from:
     # -DILI9488_DRIVER=1
     # To:
     -DST7796_DRIVER=1
   ```

2. (Optional) Change board types to use Sunton definitions:
   ```ini
   [env:cyd28]
   board = esp32-2432S028R  # From boards/ submodule
   ```

3. Test on all 3 board variants

**Recommendation**: ✅ **RECOMMENDED** - Fixes potential driver mismatch issue

---

### Option C: Keep Current Implementation ✅ ZERO EFFORT

**Scope:**
- No changes to codebase
- Current implementation appears functional

**Benefits:**
- ✅ Zero development time
- ✅ Zero risk of bugs
- ✅ Proven working implementation
- ✅ Touch calibration handles orientation differences

**Questions for Issue Author:**
1. Are there actual bugs with display orientation on cyd28/cyd24/cyd35?
2. Are there actual bugs with touchscreen accuracy?
3. Does touch calibration resolve any issues?

**Recommendation**: ✅ **RECOMMENDED** if no bugs exist

---

## Board Definitions Value

Even without using the library, the board definitions provide valuable reference:

### ESP32-2432S028R (cyd28 - 2.8" ILI9341)
```json
{
  "DISPLAY_WIDTH": 240,
  "DISPLAY_HEIGHT": 320,
  "DISPLAY_BCKL": 21,
  "DISPLAY_ILI9341_SPI": true,
  "DISPLAY_MIRROR_X": true,
  "DISPLAY_MIRROR_Y": false,
  "DISPLAY_SWAP_XY": false,
  "TOUCH_XPT2046_SPI": true,
  "TOUCH_MIRROR_X": true,
  "TOUCH_MIRROR_Y": false,
  "TOUCH_SWAP_XY": false,
  "TFT_BL": 21,
  "TFT_MOSI": 13,
  "TFT_MISO": 12,
  "TFT_SCLK": 14,
  "TFT_CS": 15,
  "TFT_DC": 2
}
```

### ESP32-2432S024R (cyd24 - 2.4" ILI9341)
```json
{
  "DISPLAY_WIDTH": 240,
  "DISPLAY_HEIGHT": 320,
  "DISPLAY_BCKL": 27,  // ← Different backlight pin!
  "DISPLAY_ILI9341_SPI": true,
  "DISPLAY_MIRROR_X": true,
  "DISPLAY_MIRROR_Y": false,
  "DISPLAY_SWAP_XY": false,
  // Touch settings same as 2.8"
  "TFT_BL": 27  // ← Different backlight pin!
}
```

### ESP32-3248S035R (cyd35 - 3.5" ST7796/ILI9488)
```json
{
  "DISPLAY_WIDTH": 320,
  "DISPLAY_HEIGHT": 480,
  "DISPLAY_BCKL": 27,
  "DISPLAY_ST7796_SPI": true,  // ← Different driver!
  "DISPLAY_MIRROR_X": true,
  "DISPLAY_MIRROR_Y": false,
  "DISPLAY_SWAP_XY": false,
  // Touch settings same as 2.8"
}
```

**Current platformio.ini comparison:**
- ✅ Pin configuration matches board definitions
- ✅ Backlight pins correct (21 for cyd28, 27 for cyd24/cyd35)
- ⚠️ **Driver mismatch for cyd35**: platformio.ini uses ILI9488_DRIVER, but board definition specifies ST7796_SPI
  - **Action Required**: Test if ST7796 driver should be used instead
  - ST7796 and ILI9488 are similar controllers but not identical
  - This may explain potential orientation issues on cyd35 if they exist

## Issues Referenced

### julesdg6/CYD-MIDEE#75 - Display Orientation
**Claimed Fix**: Library handles rotation automatically  
**Reality**: Library requires LVGL migration  
**Potential Root Cause**: Driver mismatch on cyd35 (ILI9488 vs ST7796)  
**Current Status**: Need to verify if this is actually broken

### julesdg6/CYD-MIDEE#37 - Touchscreen Compatibility
**Claimed Fix**: Library supports XPT2046 and GT911  
**Reality**: Current code only supports XPT2046 (resistive), all "R" variants have XPT2046  
**Current Status**: Not an issue unless adding "C" (capacitive) variant support

## Potential Issues Found

### CYD 3.5" (cyd35) - Driver Mismatch ⚠️

**Current platformio.ini:**
```ini
-DILI9488_DRIVER=1
```

**Board definition (esp32-3248S035R.json):**
```json
"DISPLAY_ST7796_SPI"
```

**Impact**: 
- ILI9488 and ST7796 are different display controllers
- Using wrong driver may cause orientation, color, or rendering issues
- This could be the root cause of issue #75 if it exists

**Fix (Option B):**
```ini
# Change from:
-DILI9488_DRIVER=1

# To:
-DST7796_DRIVER=1
```

**Testing Required**: Verify this doesn't break existing cyd35 deployments

## Conclusion

**Primary Finding**: Cannot use esp32-smartdisplay library without full LVGL migration.

**Secondary Finding**: Identified potential driver mismatch for cyd35 (ILI9488 vs ST7796).

**Recommendations**:
1. **First, try Option B**: Fix cyd35 driver mismatch (1-2 hours, low risk)
2. **If no bugs exist after fix**: Keep current implementation (Option C)
3. **If major UI redesign desired**: Full LVGL migration (Option A, 8-12 weeks)

**Action Required**: Issue author should:
- Test current cyd35 implementation for display issues
- If issues exist, try ST7796_DRIVER fix (Option B)
- Clarify if LVGL migration is desired (Option A)

---

## References

- **Library GitHub**: https://github.com/rzeldent/esp32-smartdisplay
- **Board Definitions**: https://github.com/rzeldent/platformio-espressif32-sunton
- **LVGL Documentation**: https://docs.lvgl.io/
- **Current TFT_eSPI**: https://github.com/Bodmer/TFT_eSPI
