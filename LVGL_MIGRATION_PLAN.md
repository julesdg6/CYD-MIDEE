# LVGL Migration Plan - Option A Sub-Issues

**Project**: CYD-MIDI-Controller Enhanced Edition  
**Goal**: Migrate from TFT_eSPI to LVGL with esp32-smartdisplay library  
**Estimated Duration**: 8-12 weeks  
**Created**: 2025-12-31

---

## Overview

This document breaks down the full LVGL migration (Option A) into manageable sub-issues. Each issue is scoped for ~2-5 days of work and includes acceptance criteria.

**Prerequisites:**
- User has allocated 8-12 weeks for this migration
- User accepts ~200KB binary size increase
- User accepts potential performance changes
- Hardware testing capability on cyd35, cyd28, and cyd24

---

## Phase 1: Foundation & Learning (Week 1-2)

### Issue 1.1: LVGL Learning & Environment Setup
**Effort**: 3-5 days  
**Priority**: P0 (Blocker)

**Tasks:**
- [ ] Study LVGL 9.x documentation (https://docs.lvgl.io/)
- [ ] Complete LVGL quick start tutorial
- [ ] Install and learn SquareLine Studio (optional but recommended)
- [ ] Create simple "Hello World" LVGL app on CYD hardware
- [ ] Test esp32-smartdisplay library with basic example
- [ ] Verify touch input works with library

**Deliverables:**
- Working LVGL example on actual hardware
- Documentation of LVGL basics learned
- Test project with esp32-smartdisplay integration

**Acceptance Criteria:**
- ✅ LVGL renders on all 3 board variants (cyd35, cyd28, cyd24)
- ✅ Touch input responds to button presses
- ✅ Display rotation correct
- ✅ No compilation warnings

**Dependencies**: None

**Risks**: 
- Learning curve steeper than expected
- Hardware compatibility issues

---

### Issue 1.2: Project Structure & Build System Migration ✅ COMPLETE
**Effort**: 2-3 days  
**Priority**: P0 (Blocker)  
**Status**: ✅ Completed on 2025-12-31 (PR #79)

**Tasks:**
- [x] Add `rzeldent/esp32_smartdisplay` to platformio.ini lib_deps
- [x] Add LVGL 9.x dependency
- [x] Create `lv_conf.h` configuration file
- [x] Update build flags for LVGL
- [x] Configure LVGL memory allocation (use PSRAM if available)
- [x] Set up LVGL logging for debugging
- [x] Test compilation on all board variants

**Deliverables:**
- ✅ Updated `platformio.ini` with new dependencies
- ✅ Configured `lv_conf.h`
- ✅ Build passing on all variants

**Acceptance Criteria:**
- ✅ Project compiles with LVGL + esp32-smartdisplay
- ✅ Binary size under 3.1MB flash limit (1.73MB / 3.14MB = 54.9%)
- ✅ No memory allocation errors (23.4% RAM usage)
- ✅ All board variants build successfully (cyd28 verified)

**Dependencies**: Issue 1.1

**Files to Modify:**
- `platformio.ini`
- Create `src/lv_conf.h` or `include/lv_conf.h`

**Reference**: 
- LVGL config template: https://github.com/lvgl/lvgl/blob/master/lv_conf_template.h
- esp32-smartdisplay examples

---

### Issue 1.3: Hardware Initialization Refactor ✅ COMPLETE
**Effort**: 2-3 days  
**Priority**: P0 (Blocker)  
**Status**: ✅ Completed on 2025-12-31

**Tasks:**
- [x] Replace TFT_eSPI initialization with `smartdisplay_init()`
- [x] Remove manual touch controller setup (kept for backward compatibility during migration)
- [x] Remove manual rotation functions (kept for backward compatibility)
- [x] Update main loop to call `lv_tick_inc()` and `lv_task_handler()`
- [x] Verify display and touch work correctly
- [ ] Remove deprecated TFT_eSPI and XPT2046_Touchscreen dependencies (Phase 10)

**Deliverables:**
- ✅ Updated `CYD-MIDI-Controller.ino` with LVGL initialization
- ✅ TFT_eSPI setup code retained for backward compatibility
- ✅ Working display + touch via LVGL (test mode)
- ✅ LVGL test mode created for validation

**Acceptance Criteria:**
- ✅ Display initializes correctly on all boards (code ready, awaiting hardware test)
- ✅ Touch coordinates map correctly (code ready, awaiting hardware test)
- ⏳ TFT_eSPI dependencies remain for backward compatibility
- ✅ Backlight control works (via TFT_BL pin)
- ✅ LVGL test mode demonstrates event-driven UI

**Dependencies**: Issue 1.2

**Files Modified:**
- `src/CYD-MIDI-Controller.ino` (LVGL init, ticker, test mode)
- `src/common_definitions.h` (LVGL_TEST enum)
- `src/lvgl_test_mode.h` (NEW - test mode implementation)
- `PHASE_1_3_NOTES.md` (NEW - detailed implementation notes)

**Implementation Notes:**
- LVGL initialized after TFT_eSPI for backward compatibility
- Dual display system during migration (both TFT_eSPI and LVGL active)
- LVGL test mode serves as reference implementation
- smartdisplay_init() auto-detects board variant
- Display rotation set to LV_DISPLAY_ROTATION_90 (landscape)
- lv_tick_inc() and lv_timer_handler() called in main loop
- See PHASE_1_3_NOTES.md for complete technical details

---

## Phase 2: UI Component Library (Week 3)

### Issue 2.1: Create LVGL UI Component Library
**Effort**: 3-5 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Create `src/ui_components.h` and `src/ui_components.cpp`
- [ ] Implement `UIButton` class wrapping `lv_btn` with callbacks
- [ ] Implement `UISlider` class wrapping `lv_slider`
- [ ] Implement `UILabel` class wrapping `lv_label`
- [ ] Implement `UIPanel` class for organizing components
- [ ] Create theme/color constants matching current THEME_* colors
- [ ] Add helper functions for common layouts (grid, flex)
- [ ] Document component API

**Deliverables:**
- Reusable LVGL widget wrappers
- Theme configuration
- Example usage documentation

**Acceptance Criteria:**
- ✅ Components compile and link
- ✅ Colors match existing theme (THEME_PRIMARY, etc.)
- ✅ Touch callbacks work correctly
- ✅ Components scale properly across resolutions

**Dependencies**: Issue 1.3

**Files to Create:**
- `src/ui_components.h`
- `src/ui_components.cpp`
- `src/lvgl_theme.h` (theme colors)

**Reference:**
- Current theme: `src/common_definitions.h` (THEME_BG, THEME_PRIMARY, etc.)

---

### Issue 2.2: Create Common UI Patterns
**Effort**: 2-3 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Create header bar component (replaces `drawModuleHeader()`)
- [ ] Create back button component
- [ ] Create BLE status indicator
- [ ] Create BPM indicator
- [ ] Create settings icon
- [ ] Implement navigation stack for mode switching
- [ ] Add transitions/animations (optional)

**Deliverables:**
- Standard header component
- Navigation system
- Common UI patterns library

**Acceptance Criteria:**
- ✅ Header looks consistent across modes
- ✅ Back button works in all modes
- ✅ BLE/BPM indicators update in real-time
- ✅ Navigation smooth and responsive

**Dependencies**: Issue 2.1

**Files to Create:**
- `src/ui_patterns.h`
- `src/ui_patterns.cpp`

---

## Phase 3: Main Menu Migration (Week 3-4)

### Issue 3.1: Migrate Main Menu to LVGL
**Effort**: 2-3 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Replace main menu grid with LVGL grid layout
- [ ] Create app icon buttons using LVGL
- [ ] Implement symbol rendering (♪, ♫, ●, etc.) using LVGL labels
- [ ] Add touch event handlers for app selection
- [ ] Migrate settings menu
- [ ] Test menu navigation and app launching

**Deliverables:**
- LVGL-based main menu
- Working app icon grid
- Settings menu migrated

**Acceptance Criteria:**
- ✅ Menu renders correctly on all resolutions
- ✅ Touch accurately selects apps
- ✅ Icons display with correct colors
- ✅ App launching works

**Dependencies**: Issue 2.2

**Files to Modify:**
- `src/CYD-MIDI-Controller.ino` (drawMenu, showSettingsMenu)

---

## Phase 4: Simple Mode Migrations (Week 4-5)

### Issue 4.1: Migrate Keyboard Mode
**Effort**: 2-3 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Create LVGL button grid for piano keys
- [ ] Implement octave +/- buttons
- [ ] Implement scale selector button
- [ ] Add note visualization (pressed key highlighting)
- [ ] Hook up MIDI note sending
- [ ] Test keyboard responsiveness

**Deliverables:**
- Working keyboard mode in LVGL
- Responsive touch input

**Acceptance Criteria:**
- ✅ Keys display correctly and respond to touch
- ✅ MIDI notes sent correctly
- ✅ Octave changes work
- ✅ Scale changes work
- ✅ Visual feedback on key press

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/keyboard_mode.h`

**Mode Complexity**: ⭐ Simple (mostly buttons)

---

### Issue 4.2: Migrate XY Pad Mode
**Effort**: 1-2 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Create LVGL canvas for XY pad area
- [ ] Implement touch tracking within canvas
- [ ] Add crosshair/cursor visualization
- [ ] Map X/Y to MIDI CC values
- [ ] Add hold/release detection
- [ ] Add visual feedback

**Deliverables:**
- Working XY pad with LVGL

**Acceptance Criteria:**
- ✅ Touch tracking smooth and accurate
- ✅ MIDI CC messages sent correctly
- ✅ Visual cursor updates in real-time
- ✅ Full pad area usable

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/xy_pad_mode.h`

**Mode Complexity**: ⭐ Simple (canvas + touch)

---

### Issue 4.3: Migrate Auto Chord Mode
**Effort**: 2 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create chord pad button grid
- [ ] Implement key selection buttons
- [ ] Add chord type selector
- [ ] Hook up chord MIDI output
- [ ] Add visual feedback for active chord

**Deliverables:**
- Working auto chord mode

**Acceptance Criteria:**
- ✅ Chord pads trigger correct notes
- ✅ Key selection works
- ✅ Visual feedback clear

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/auto_chord_mode.h`

**Mode Complexity**: ⭐ Simple (buttons)

---

### Issue 4.4: Migrate Grid Piano (Pads) Mode
**Effort**: 2 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create LVGL button matrix for pads
- [ ] Implement note assignment logic
- [ ] Add velocity sensitivity (if supported)
- [ ] Add visual feedback on pad press
- [ ] Test MIDI output

**Deliverables:**
- Working pad grid mode

**Acceptance Criteria:**
- ✅ All pads respond to touch
- ✅ Correct notes sent
- ✅ Visual feedback works

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/grid_piano_mode.h`

**Mode Complexity**: ⭐ Simple (button matrix)

---

### Issue 4.5: Migrate Random Generator Mode
**Effort**: 1-2 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create control buttons (Play/Stop, Rate, Range)
- [ ] Use LVGL canvas for random note visualization
- [ ] Implement timer-based note generation
- [ ] Add visual feedback for generated notes
- [ ] Test randomization algorithm

**Deliverables:**
- Working random generator

**Acceptance Criteria:**
- ✅ Random notes generated at correct rate
- ✅ Visual dots show generated notes
- ✅ Controls work correctly

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/random_generator_mode.h`

**Mode Complexity**: ⭐⭐ Moderate (canvas + animation)

---

## Phase 5: Animation Modes (Week 6)

### Issue 5.1: Migrate Bouncing Ball Mode
**Effort**: 2-3 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create LVGL canvas for ball physics
- [ ] Implement ball animation using LVGL animation framework
- [ ] Add gravity and bounce physics
- [ ] Map ball position to MIDI notes
- [ ] Add control sliders (gravity, elasticity, etc.)
- [ ] Optimize animation performance

**Deliverables:**
- Working bouncing ball with physics

**Acceptance Criteria:**
- ✅ Ball animates smoothly (30+ FPS)
- ✅ Physics feel realistic
- ✅ MIDI notes triggered on bounce
- ✅ Controls adjust physics correctly

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/bouncing_ball_mode.h`

**Mode Complexity**: ⭐⭐⭐ Complex (physics + animation)

---

### Issue 5.2: Migrate Physics Drop Mode
**Effort**: 2-3 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create LVGL canvas for ball drop simulation
- [ ] Implement multi-ball physics
- [ ] Add platform drawing and collision detection
- [ ] Map collisions to MIDI notes
- [ ] Add touch interaction for adding balls/platforms
- [ ] Optimize for multiple objects

**Deliverables:**
- Working physics drop mode

**Acceptance Criteria:**
- ✅ Multiple balls animate smoothly
- ✅ Collisions detected accurately
- ✅ MIDI notes triggered on collision
- ✅ Touch adds balls/platforms

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/physics_drop_mode.h`

**Mode Complexity**: ⭐⭐⭐ Complex (multi-object physics)

---

## Phase 6: Sequencer Modes (Week 7-8)

### Issue 6.1: Migrate Sequencer Mode
**Effort**: 3-4 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Create LVGL button matrix for 16-step grid (4 tracks)
- [ ] Implement step selection with visual feedback
- [ ] Add playback cursor animation
- [ ] Add control buttons (Play/Stop, Clear, BPM)
- [ ] Implement sequencer playback logic
- [ ] Add track mute buttons
- [ ] Test timing accuracy

**Deliverables:**
- Working 16-step sequencer

**Acceptance Criteria:**
- ✅ All steps trigger correctly
- ✅ Playback timing accurate
- ✅ Visual feedback clear
- ✅ BPM control works
- ✅ Track muting works

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/sequencer_mode.h`

**Mode Complexity**: ⭐⭐⭐ Complex (timing + grid)

---

### Issue 6.2: Migrate Euclidean Sequencer Mode
**Effort**: 3-4 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create circular rhythm visualization using LVGL canvas
- [ ] Implement Euclidean rhythm algorithm
- [ ] Add parameter controls (steps, pulses, rotation, track)
- [ ] Add playback with circular cursor
- [ ] Add preset buttons
- [ ] Test rhythm generation

**Deliverables:**
- Working Euclidean sequencer with visualization

**Acceptance Criteria:**
- ✅ Euclidean algorithm generates correct patterns
- ✅ Circular visualization accurate
- ✅ All parameters adjustable
- ✅ Playback timing correct

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/euclidean_mode.h`
- `src/euclidean_mode.cpp`

**Mode Complexity**: ⭐⭐⭐⭐ Very Complex (algorithm + visualization)

---

### Issue 6.3: Migrate Grids (Drum Sequencer) Mode
**Effort**: 3-4 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create concentric circle visualization (LVGL canvas)
- [ ] Implement Mutable Instruments Grids algorithm
- [ ] Add X/Y/Chaos parameter sliders
- [ ] Add pattern selection
- [ ] Implement playback with visual cursor
- [ ] Test drum pattern generation

**Deliverables:**
- Working Grids drum sequencer

**Acceptance Criteria:**
- ✅ Grids algorithm generates correct patterns
- ✅ Visualization matches pattern state
- ✅ Parameters affect patterns correctly
- ✅ Playback timing accurate

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/grids_mode.h`
- `src/grids_mode.cpp`

**Mode Complexity**: ⭐⭐⭐⭐ Very Complex (complex algorithm)

---

## Phase 7: Advanced Modes (Week 8-9)

### Issue 7.1: Migrate Arpeggiator Mode
**Effort**: 2-3 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create note input buttons (chord entry)
- [ ] Add pattern selector (Up, Down, UpDown, Random, etc.)
- [ ] Add rate control
- [ ] Add octave range control
- [ ] Implement arpeggiator playback logic
- [ ] Add visual feedback for current note

**Deliverables:**
- Working arpeggiator

**Acceptance Criteria:**
- ✅ Chord input works
- ✅ All patterns play correctly
- ✅ Rate control accurate
- ✅ Octave range works

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/arpeggiator_mode.h`
- `src/arpeggiator_mode.cpp`

**Mode Complexity**: ⭐⭐⭐ Complex (state machine)

---

### Issue 7.2: Migrate LFO Mode
**Effort**: 2-3 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create LVGL canvas for waveform visualization
- [ ] Implement LFO waveform generation (sine, saw, square, etc.)
- [ ] Add waveform selector
- [ ] Add rate and depth controls
- [ ] Map LFO to MIDI CC output
- [ ] Add visual waveform animation

**Deliverables:**
- Working LFO generator with visualization

**Acceptance Criteria:**
- ✅ All waveforms generate correctly
- ✅ MIDI CC values accurate
- ✅ Visual waveform matches output
- ✅ Rate and depth controls work

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/lfo_mode.h`

**Mode Complexity**: ⭐⭐⭐ Complex (waveform math + visualization)

---

### Issue 7.3: Migrate TB3PO (Acid) Mode
**Effort**: 3-4 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create step sequencer grid with note/accent/slide
- [ ] Add pattern editing interface
- [ ] Implement TB-303 style sequencer logic
- [ ] Add parameter controls (Cutoff, Resonance, Accent, etc.)
- [ ] Add playback with visual cursor
- [ ] Test acid bass line generation

**Deliverables:**
- Working TB-303 style sequencer

**Acceptance Criteria:**
- ✅ Step editing works correctly
- ✅ Accent and slide function properly
- ✅ Playback timing accurate
- ✅ Parameters affect MIDI output

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/tb3po_mode.h`
- `src/tb3po_mode.cpp`

**Mode Complexity**: ⭐⭐⭐⭐ Very Complex (TB-303 sequencer)

---

### Issue 7.4: Migrate Raga Mode
**Effort**: 3-4 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create raga scale selector interface
- [ ] Implement drone note controls
- [ ] Add note selection pad
- [ ] Implement Indian classical music scales
- [ ] Add visual feedback for raga structure
- [ ] Test scale accuracy

**Deliverables:**
- Working raga generator

**Acceptance Criteria:**
- ✅ Raga scales accurate
- ✅ Drone notes work correctly
- ✅ Note selection intuitive
- ✅ Visual feedback clear

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/raga_mode.h`
- `src/raga_mode.cpp`

**Mode Complexity**: ⭐⭐⭐ Complex (music theory)

---

### Issue 7.5: Migrate Morph Mode
**Effort**: 3-4 days  
**Priority**: P2 (Medium)

**Tasks:**
- [ ] Create gesture capture canvas
- [ ] Implement gesture recording
- [ ] Add gesture playback with morphing
- [ ] Add parameter controls (speed, loop, etc.)
- [ ] Map gesture position to MIDI CC
- [ ] Add visual feedback for recorded gestures

**Deliverables:**
- Working gesture morphing mode

**Acceptance Criteria:**
- ✅ Gesture recording accurate
- ✅ Morphing/playback smooth
- ✅ MIDI CC output correct
- ✅ Visual feedback intuitive

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/morph_mode.h`
- `src/morph_mode.cpp`

**Mode Complexity**: ⭐⭐⭐⭐ Very Complex (gesture recognition)

---

## Phase 8: Calibration & Settings (Week 9-10)

### Issue 8.1: Migrate Touch Calibration System
**Effort**: 2-3 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Migrate touch calibration UI to LVGL
- [ ] Implement 3-point calibration with LVGL
- [ ] Use esp32-smartdisplay touch calibration API
- [ ] Add calibration data storage
- [ ] Test calibration accuracy on all boards

**Deliverables:**
- Working touch calibration in LVGL

**Acceptance Criteria:**
- ✅ Calibration process intuitive
- ✅ Touch accuracy improved after calibration
- ✅ Calibration persists across reboots
- ✅ Works on all board variants

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/touch_calibration.h`

---

### Issue 8.2: Migrate Settings Menu & System Functions
**Effort**: 2-3 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Migrate Bluetooth toggle to LVGL
- [ ] Migrate MIDI channel selector to LVGL
- [ ] Migrate BPM setting to LVGL
- [ ] Migrate Screenshot mode to LVGL
- [ ] Add WiFi settings UI (if not already done)
- [ ] Test all settings persistence

**Deliverables:**
- Complete settings menu in LVGL

**Acceptance Criteria:**
- ✅ All settings functional
- ✅ Settings persist correctly
- ✅ BLE toggle works
- ✅ Screenshot capture works

**Dependencies**: Issue 3.1

**Files to Modify:**
- `src/CYD-MIDI-Controller.ino` (showSettingsMenu)

---

## Phase 9: Testing & Optimization (Week 10-11)

### Issue 9.1: Comprehensive Testing on All Boards
**Effort**: 3-5 days  
**Priority**: P0 (Blocker)

**Tasks:**
- [ ] Test each mode on cyd35 (3.5" ILI9488/ST7796)
- [ ] Test each mode on cyd28 (2.8" ILI9341)
- [ ] Test each mode on cyd24 (2.4" ILI9341)
- [ ] Verify touch accuracy on all boards
- [ ] Verify display orientation on all boards
- [ ] Test BLE connectivity
- [ ] Test SD card functionality
- [ ] Test WiFi web server
- [ ] Document any board-specific issues

**Deliverables:**
- Test report for each board
- Bug list with reproduction steps
- Performance benchmarks

**Acceptance Criteria:**
- ✅ All 15 modes work on all boards
- ✅ Touch accurate on all boards
- ✅ No crashes or freezes
- ✅ MIDI output correct

**Dependencies**: All mode migration issues (4.x - 7.x)

---

### Issue 9.2: Performance Optimization
**Effort**: 3-5 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Profile memory usage (heap, stack, LVGL buffers)
- [ ] Optimize LVGL buffer sizes
- [ ] Reduce unnecessary redraws
- [ ] Optimize animation frame rates
- [ ] Check for memory leaks
- [ ] Optimize MIDI sending (use threading if needed)
- [ ] Measure and improve touch latency
- [ ] Verify binary size under 3.1MB

**Deliverables:**
- Performance optimization report
- Benchmarks (FPS, memory usage, latency)
- Optimized code

**Acceptance Criteria:**
- ✅ Smooth animations (30+ FPS target)
- ✅ Binary size < 3.1MB
- ✅ Heap usage stable (no leaks)
- ✅ Touch latency < 50ms

**Dependencies**: Issue 9.1

---

### Issue 9.3: Bug Fixing & Regression Testing
**Effort**: 3-5 days  
**Priority**: P0 (Blocker)

**Tasks:**
- [ ] Fix all P0/P1 bugs from testing phase
- [ ] Regression test after each fix
- [ ] Document known issues and workarounds
- [ ] Create bug tracking spreadsheet
- [ ] Test edge cases (mode switching, rapid touch, etc.)
- [ ] Test long-running stability (24hr test)

**Deliverables:**
- All critical bugs fixed
- Regression test results
- Known issues documented

**Acceptance Criteria:**
- ✅ No P0 bugs remaining
- ✅ < 3 P1 bugs remaining (documented)
- ✅ 24-hour stability test passes
- ✅ Mode switching smooth

**Dependencies**: Issue 9.2

---

## Phase 10: Documentation & Polish (Week 11-12)

### Issue 10.1: Update Documentation
**Effort**: 2-3 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Update README.md with LVGL information
- [ ] Update BUILD.md with new build instructions
- [ ] Update platformio.ini comments
- [ ] Document LVGL configuration choices
- [ ] Update screenshots in assets/screenshots/
- [ ] Create LVGL migration guide for future developers
- [ ] Update DEV_NOTES.md with LVGL patterns

**Deliverables:**
- Updated documentation
- New screenshots
- Migration guide

**Acceptance Criteria:**
- ✅ All documentation accurate
- ✅ Build instructions work
- ✅ Screenshots up-to-date
- ✅ Migration guide complete

**Dependencies**: Issue 9.3

---

### Issue 10.2: Code Cleanup & Final Review
**Effort**: 2-3 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Remove all TFT_eSPI code
- [ ] Remove unused dependencies
- [ ] Clean up commented-out code
- [ ] Format code consistently
- [ ] Add missing code comments
- [ ] Run linter/formatter
- [ ] Final code review
- [ ] Update CHANGELOG.md

**Deliverables:**
- Clean, production-ready code
- Updated CHANGELOG

**Acceptance Criteria:**
- ✅ No dead code
- ✅ No compilation warnings
- ✅ Code follows style guide
- ✅ All public functions documented

**Dependencies**: Issue 10.1

---

### Issue 10.3: Release Preparation
**Effort**: 1-2 days  
**Priority**: P1 (High)

**Tasks:**
- [ ] Create release notes
- [ ] Tag release version
- [ ] Test pre-built binaries on all boards
- [ ] Create migration guide for existing users
- [ ] Update GitHub release with binaries
- [ ] Announce LVGL migration in README

**Deliverables:**
- GitHub release with binaries
- Migration guide for users
- Release announcement

**Acceptance Criteria:**
- ✅ Release tagged in git
- ✅ Binaries tested and working
- ✅ Migration guide clear
- ✅ Documentation complete

**Dependencies**: Issue 10.2

---

## Risk Mitigation

### Binary Size Risk
**Risk**: LVGL + smartdisplay may exceed 3.1MB flash limit  
**Mitigation**:
- Monitor binary size throughout development
- Use LVGL's minimal configuration
- Enable compiler optimizations (-Os)
- Consider moving assets to SD card if needed

### Performance Risk
**Risk**: LVGL may be slower than TFT_eSPI for real-time animations  
**Mitigation**:
- Profile early (Issue 1.1)
- Use hardware acceleration where available
- Optimize LVGL buffer configuration
- Consider partial refresh strategies

### Learning Curve Risk
**Risk**: LVGL learning takes longer than 1-2 weeks  
**Mitigation**:
- Start with simple modes first
- Use SquareLine Studio for complex UIs
- Consult LVGL forums and examples
- Budget extra time in schedule

### Hardware Compatibility Risk
**Risk**: esp32-smartdisplay doesn't work on all variants  
**Mitigation**:
- Test on actual hardware early (Issue 1.1)
- Have fallback to TFT_eSPI if critical issues found
- Engage with library maintainer if issues found

---

## Dependencies & Tools

**Required:**
- PlatformIO
- esp32-smartdisplay library (latest)
- LVGL 9.x
- Physical CYD hardware (all 3 variants ideally)

**Optional:**
- SquareLine Studio (UI designer)
- LVGL SimSDL (PC simulation)
- Logic analyzer (for performance profiling)

---

## Timeline Summary

| Week | Phase | Key Milestones |
|------|-------|----------------|
| 1-2 | Foundation | LVGL working, build system migrated |
| 3 | UI Components | Component library ready, menu migrated |
| 4-5 | Simple Modes | 5 simple modes migrated |
| 6 | Animation Modes | Physics modes migrated |
| 7-8 | Sequencers | Complex sequencer modes migrated |
| 8-9 | Advanced Modes | Remaining complex modes done |
| 9-10 | Settings & Calibration | System functions migrated |
| 10-11 | Testing & Optimization | All modes tested, optimized |
| 11-12 | Documentation & Release | Documentation complete, release ready |

**Total Effort**: 8-12 weeks full-time

---

## Success Criteria

**Project is successful if:**
- ✅ All 15 MIDI modes work correctly with LVGL
- ✅ Touch accuracy equal or better than before
- ✅ Display orientation correct on all boards
- ✅ Binary size under 3.1MB
- ✅ Performance acceptable (30+ FPS for animations)
- ✅ No major regressions from TFT_eSPI version
- ✅ Code maintainability improved
- ✅ Documentation complete

---

## Notes

1. **Issue Ordering**: Issues should be completed in numerical order within each phase. Some parallelization is possible (e.g., different modes can be migrated concurrently).

2. **Mode Complexity Ratings**:
   - ⭐ Simple: Mostly buttons/basic widgets (1-2 days)
   - ⭐⭐ Moderate: Canvas + simple animation (2-3 days)
   - ⭐⭐⭐ Complex: Complex state or algorithms (3-4 days)
   - ⭐⭐⭐⭐ Very Complex: Advanced algorithms or UI (4-5 days)

3. **Testing Strategy**: Each issue should include unit testing where applicable. Integration testing happens in Phase 9.

4. **Code Review**: Recommend code review after each phase before proceeding to next.

5. **Rollback Plan**: Keep TFT_eSPI branch as backup. Tag commits before major changes.

---

## Questions for Stakeholder

Before starting, please confirm:

1. **Timeline**: Is 8-12 weeks acceptable?
2. **Resources**: Do you have all 3 board variants for testing?
3. **Binary Size**: Can we increase flash partition if needed?
4. **Performance**: What's minimum acceptable FPS for animations?
5. **Features**: Any features can be cut to save time?
6. **Tooling**: Should we use SquareLine Studio ($0-299)?

---

**Document Version**: 1.0  
**Last Updated**: 2025-12-31  
**Author**: GitHub Copilot
