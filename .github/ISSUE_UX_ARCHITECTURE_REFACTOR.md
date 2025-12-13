# Issue: UX Architecture Refactor - Centralized Event-Driven UI System

**Priority:** HIGH  
**Type:** Architecture / Refactor  
**Affects:** All 15 modes, touch handling, UI components  
**Status:** Proposed  

---

## Problem Statement

### Touch Responsiveness Issues
1. **Inconsistent responsiveness across modes:**
   - Main menu is responsive everywhere except the cog icon
   - Module screens have poor touch responsiveness throughout
   - Some buttons require multiple touches to register

2. **Root causes identified:**
   - Touch handling logic scattered across 15+ mode files
   - Each mode implements its own button checking (duplicated code)
   - Direct coordinate checking in `handleXMode()` functions
   - Mix of hardcoded pixel values and calculated layouts
   - No consistent touch debouncing or timing strategy
   - Overlapping UI components in some modes (e.g., Raga slider overlapped by buttons)

### Current Architecture Problems

```
Current Flow (BROKEN):
┌─────────────────┐
│  loop()         │ → updateTouch() (global)
│  CYD-MIDI-      │
│  Controller.ino │
└────────┬────────┘
         │
         ├──→ handleKeyboardMode() ──→ if (touch.justPressed && x>=... && x<=...)
         ├──→ handleSequencerMode() ──→ if (touch.justPressed && x>=... && x<=...)
         ├──→ handleRagaMode()      ──→ if (touch.justPressed && x>=... && x<=...)
         ├──→ handleTB3POMode()     ──→ if (touch.justPressed && x>=... && x<=...)
         └──→ ... (12 more modes, each with own touch logic)

ISSUES:
✗ Touch logic duplicated 15 times
✗ Different timing/debouncing in each mode  
✗ No centralized event dispatch
✗ Hardcoded coordinates mixed with calculated layouts
✗ UI components not reusable
✗ Overlapping components not detected
✗ Button feedback inconsistent (some use isPressed, some don't)
```

---

## Proposed Solution: Event-Driven UI Architecture

### Architecture Overview

```
Proposed Flow (EVENT-DRIVEN):
┌─────────────────────────────────────────────────────┐
│  loop() - CYD-MIDI-Controller.ino                   │
│                                                      │
│  1. updateTouch() - Read hardware                   │
│  2. UIManager::processEvents() - Dispatch events    │
│     │                                                │
│     └──→ UIManager checks all registered components │
│          for current mode                           │
│          │                                           │
│          ├──→ Button::checkEvent() → callback()     │
│          ├──→ Slider::checkEvent() → callback()     │
│          ├──→ Grid::checkEvent()   → callback()     │
│          └──→ Pad::checkEvent()    → callback()     │
└─────────────────────────────────────────────────────┘
         ↓
┌─────────────────────────────────────────────────────┐
│  Mode Files (keyboard_mode.cpp, raga_mode.cpp, etc)│
│                                                      │
│  • initializeMode() - Register UI components        │
│  • onButtonPress() - Handle button events           │
│  • onSliderChange() - Handle slider events          │
│  • updateAnimation() - Render animations/feedback   │
│                                                      │
│  NO DIRECT TOUCH HANDLING IN MODES                  │
└─────────────────────────────────────────────────────┘

BENEFITS:
✓ Single touch processing loop (consistent timing)
✓ UI components are library objects
✓ Event callbacks in modes (clean separation)
✓ Automatic overlap detection
✓ Centralized touch debouncing
✓ Consistent button feedback
✓ Calculated layouts only (no hardcoded coords)
```

---

## Detailed Requirements

### 1. Create UIManager Class (`ui_manager.h` / `ui_manager.cpp`)

**Responsibilities:**
- Maintain registry of all UI components for current mode
- Process touch events once per frame
- Dispatch events to registered components
- Detect and prevent overlapping components
- Handle z-ordering (which component is on top)
- Provide layout helpers (grid, flex, absolute positioning)

**API:**
```cpp
class UIManager {
public:
  // Lifecycle
  static void init();
  static void clearMode(); // Clear all components for mode switch
  
  // Component registration
  static void registerButton(Button* btn);
  static void registerSlider(Slider* slider);
  static void registerGrid(Grid* grid);
  static void registerPad(TouchPad* pad);
  
  // Event processing (called from main loop)
  static void processEvents();
  
  // Layout helpers
  static LayoutGrid createGrid(int rows, int cols, int x, int y, int w, int h, int spacing);
  static void arrangeHorizontal(UIComponent** components, int count, int y, int spacing);
  static void arrangeVertical(UIComponent** components, int count, int x, int spacing);
  
  // Validation
  static bool checkOverlaps(); // Returns true if overlaps detected
  static void debugDrawBounds(); // Visual debug mode
  
private:
  static std::vector<UIComponent*> components;
  static TouchState lastProcessedTouch;
};
```

### 2. Create Base UIComponent Class

**Base class for all interactive elements:**
```cpp
class UIComponent {
public:
  UIComponent(int x, int y, int w, int h);
  virtual ~UIComponent();
  
  // Core API
  virtual void draw(bool force = false) = 0;
  virtual bool checkEvent(const TouchState& touch) = 0;
  virtual void setBounds(int x, int y, int w, int h);
  
  // Bounds checking
  bool contains(int x, int y) const;
  bool overlaps(const UIComponent& other) const;
  Rect getBounds() const;
  
  // State
  void setEnabled(bool enabled);
  bool isEnabled() const;
  void setVisible(bool visible);
  bool isVisible() const;
  
protected:
  int x, y, w, h;
  bool enabled = true;
  bool visible = true;
  bool lastDrawnPressed = false;
};
```

### 3. Implement UI Component Library

**Button Component:**
```cpp
class Button : public UIComponent {
public:
  using Callback = std::function<void()>;
  
  Button(int x, int y, int w, int h, String text, uint16_t color = THEME_PRIMARY);
  
  void onPress(Callback callback);
  void onRelease(Callback callback);
  void setText(String text);
  void setColor(uint16_t color);
  
  void draw(bool force = false) override;
  bool checkEvent(const TouchState& touch) override;
  
private:
  String text;
  uint16_t color;
  Callback pressCallback;
  Callback releaseCallback;
  bool isPressed = false;
};
```

**Slider Component:**
```cpp
class Slider : public UIComponent {
public:
  using Callback = std::function<void(float value)>; // 0.0 - 1.0
  
  Slider(int x, int y, int w, int h, float initialValue = 0.5f);
  
  void onChange(Callback callback);
  void setValue(float value); // 0.0 - 1.0
  float getValue() const;
  void setRange(float min, float max); // For display mapping
  void setLabel(String label);
  
  void draw(bool force = false) override;
  bool checkEvent(const TouchState& touch) override;
  
private:
  String label;
  float value; // Always 0.0 - 1.0 internally
  float displayMin, displayMax; // For label formatting
  Callback changeCallback;
};
```

**Grid Component (for step sequencers, note pads):**
```cpp
class Grid : public UIComponent {
public:
  using CellCallback = std::function<void(int row, int col, bool state)>;
  
  Grid(int x, int y, int w, int h, int rows, int cols, int spacing = 2);
  
  void onCellPress(CellCallback callback);
  void setCellState(int row, int col, bool state);
  bool getCellState(int row, int col) const;
  void setCellColor(int row, int col, uint16_t color);
  void clear();
  
  void draw(bool force = false) override;
  bool checkEvent(const TouchState& touch) override;
  
private:
  int rows, cols, spacing;
  std::vector<std::vector<bool>> cellStates;
  std::vector<std::vector<uint16_t>> cellColors;
  CellCallback cellCallback;
  
  void getCellBounds(int row, int col, int& cx, int& cy, int& cw, int& ch);
};
```

**TouchPad Component (for XY control):**
```cpp
class TouchPad : public UIComponent {
public:
  using Callback = std::function<void(float x, float y)>; // 0.0 - 1.0 range
  
  TouchPad(int x, int y, int w, int h);
  
  void onTouch(Callback callback);
  void onRelease(Callback releaseCallback);
  void setPosition(float x, float y); // 0.0 - 1.0
  std::pair<float, float> getPosition() const;
  
  void draw(bool force = false) override;
  bool checkEvent(const TouchState& touch) override;
  
private:
  float currentX, currentY; // 0.0 - 1.0
  Callback touchCallback;
  Callback releaseCallback;
  bool isTouched = false;
};
```

### 4. Update All Modes to Use Event-Driven System

**Example: Raga Mode Refactored**

**Before (raga_mode.cpp - current):**
```cpp
void handleRagaMode() {
  updateTouch(); // Already called in loop!
  
  if (touch.justPressed) {
    // Check back button (hardcoded coords)
    if (isButtonPressed(10, 5, 70, 35)) {
      exitToMenu();
      return;
    }
    
    // Check raga buttons (manual loop)
    for (int i = 0; i < RAGA_COUNT; i++) {
      int x = raga.ragaBtnStartX + col * (raga.ragaBtnW + raga.ragaBtnSpacing);
      int btnY = raga.ragaBtnStartY + row * (raga.ragaBtnH + raga.ragaBtnRowSpacing);
      if (isButtonPressed(x, btnY, raga.ragaBtnW, raga.ragaBtnH)) {
        raga.currentRaga = (RagaType)i;
        drawRagaMode();
        return;
      }
    }
    
    // Check tempo slider (manual bounds check)
    if (isButtonPressed(raga.sliderX, raga.sliderY, raga.sliderW, raga.sliderH)) {
      raga.tempo = ((touch.x - raga.sliderX) * 255) / raga.sliderW;
      drawRagaMode();
      return;
    }
    
    // 4 more manual button checks...
  }
  
  // Automatic playback timing
  if (raga.playing) {
    // ...timing logic
  }
}
```

**After (raga_mode.cpp - proposed):**
```cpp
// UI components (module scope)
static Button* ragaButtons[RAGA_COUNT];
static Slider* tempoSlider;
static Button* playButton;
static Button* droneButton;
static Button* rootMinusButton;
static Button* rootPlusButton;

void initializeRagaMode() {
  Serial.println("\\n=== Raga Mode Initialization ===");
  
  // Reset state
  raga.currentRaga = BHAIRAVI;
  raga.rootNote = 60;
  raga.playing = false;
  raga.droneEnabled = false;
  raga.tempo = 128;
  
  // Clear previous UI components
  UIManager::clearMode();
  
  // Calculate layout using UIManager helpers
  int y = CONTENT_TOP + 30;
  
  // Create raga selection buttons (2 rows of 5)
  LayoutGrid grid = UIManager::createGrid(2, 5, 0, y, SCREEN_WIDTH, 110, 6);
  for (int i = 0; i < RAGA_COUNT; i++) {
    Rect cell = grid.getCell(i / 5, i % 5);
    ragaButtons[i] = new Button(cell.x, cell.y, cell.w, cell.h, 
                                 ragaScales[i].name, ragaScales[i].color);
    ragaButtons[i]->onPress([i]() {
      onRagaSelected(i);
    });
    UIManager::registerButton(ragaButtons[i]);
  }
  
  y = grid.bottom() + 30;
  
  // Scale display (static text, not interactive)
  drawScaleDisplay(y);
  y += 30;
  
  // Control section (static text)
  drawControlInfo(y);
  y += 30;
  
  // Tempo slider
  tempoSlider = new Slider(100, y, SCREEN_WIDTH - 120, 20, raga.tempo / 255.0f);
  tempoSlider->setRange(40, 200); // BPM range
  tempoSlider->setLabel("Tempo");
  tempoSlider->onChange([](float value) {
    raga.tempo = (int)(value * 255);
    updateTempoDisplay();
  });
  UIManager::registerSlider(tempoSlider);
  
  // Bottom control buttons
  int btnY = SCREEN_HEIGHT - 60;
  int btnW = (SCREEN_WIDTH - 50) / 4;
  int btnSpacing = 10;
  
  playButton = new Button(10, btnY, btnW, 50, "PLAY", THEME_PRIMARY);
  playButton->onPress([]() { togglePlay(); });
  UIManager::registerButton(playButton);
  
  droneButton = new Button(10 + btnW + btnSpacing, btnY, btnW, 50, "DRONE", THEME_SECONDARY);
  droneButton->onPress([]() { toggleDrone(); });
  UIManager::registerButton(droneButton);
  
  rootMinusButton = new Button(10 + 2 * (btnW + btnSpacing), btnY, btnW, 50, "ROOT-", THEME_ACCENT);
  rootMinusButton->onPress([]() { adjustRoot(-1); });
  UIManager::registerButton(rootMinusButton);
  
  rootPlusButton = new Button(10 + 3 * (btnW + btnSpacing), btnY, btnW, 50, "ROOT+", THEME_ACCENT);
  rootPlusButton->onPress([]() { adjustRoot(+1); });
  UIManager::registerButton(rootPlusButton);
  
  // Validate no overlaps
  if (UIManager::checkOverlaps()) {
    Serial.println("WARNING: Overlapping UI components detected!");
  }
  
  // Initial draw
  drawRagaMode();
}

void handleRagaMode() {
  // NO TOUCH HANDLING HERE - UIManager handles it
  
  // Only handle automatic playback timing
  if (raga.playing) {
    unsigned long now = millis();
    int bpm = 40 + ((raga.tempo * 160) / 255);
    unsigned long noteDelay = (60000 / bpm) / 2;
    
    if (now - raga.lastNoteTime >= noteDelay) {
      raga.lastNoteTime = now;
      playNextNote();
    }
  }
}

// Event handlers (clean, focused functions)
void onRagaSelected(int index) {
  raga.currentRaga = (RagaType)index;
  raga.currentStep = 0;
  if (raga.currentNote >= 0) {
    sendNoteOff(raga.currentNote);
    raga.currentNote = -1;
  }
  sendPitchBend(8192);
  updateRagaDisplay();
}

void togglePlay() {
  raga.playing = !raga.playing;
  if (raga.playing) {
    raga.currentStep = 0;
    raga.lastNoteTime = millis();
    playButton->setText("STOP");
  } else {
    if (raga.currentNote >= 0) {
      sendNoteOff(raga.currentNote);
      raga.currentNote = -1;
    }
    sendPitchBend(8192);
    playButton->setText("PLAY");
  }
}

void toggleDrone() {
  raga.droneEnabled = !raga.droneEnabled;
  if (raga.droneEnabled) {
    startDrone();
    droneButton->setColor(THEME_SUCCESS);
  } else {
    stopDrone();
    droneButton->setColor(THEME_SECONDARY);
  }
}

void adjustRoot(int delta) {
  if (raga.droneEnabled) stopDrone();
  raga.rootNote = constrain(raga.rootNote + delta, 36, 84);
  if (raga.droneEnabled) startDrone();
  updateRootDisplay();
}
```

### 5. Layout System Requirements

**All layouts must be calculated, never hardcoded:**

```cpp
// BAD - Hardcoded coordinates
if (isButtonPressed(10, 100, 80, 45)) { ... }
if (isButtonPressed(100, 100, 80, 45)) { ... }

// GOOD - Calculated layout
LayoutGrid grid = UIManager::createGrid(1, 3, 10, 100, SCREEN_WIDTH - 20, 45, 10);
Button* btn1 = new Button(grid.getCell(0, 0), "Button 1");
Button* btn2 = new Button(grid.getCell(0, 1), "Button 2");
Button* btn3 = new Button(grid.getCell(0, 2), "Button 3");
```

**Layout helper utilities:**
```cpp
struct Rect {
  int x, y, w, h;
  int right() const { return x + w; }
  int bottom() const { return y + h; }
  int centerX() const { return x + w / 2; }
  int centerY() const { return y + h / 2; }
};

class LayoutGrid {
public:
  LayoutGrid(int rows, int cols, int x, int y, int w, int h, int spacing);
  Rect getCell(int row, int col) const;
  int cellWidth() const;
  int cellHeight() const;
  int bottom() const;
  
private:
  int rows, cols, x, y, w, h, spacing;
};

class LayoutHelper {
public:
  // Arrange components horizontally with equal spacing
  static void arrangeHorizontal(std::vector<UIComponent*> components, 
                                 int y, int totalWidth, int spacing);
  
  // Arrange components vertically with equal spacing
  static void arrangeVertical(std::vector<UIComponent*> components, 
                               int x, int startY, int spacing);
  
  // Center a component
  static Rect centerIn(int w, int h, const Rect& container);
  
  // Calculate responsive grid (auto-size cells)
  static LayoutGrid autoGrid(int rows, int cols, int padding, int spacing);
};
```

---

## Implementation Plan

### Phase 1: Core Infrastructure (Week 1)
- [ ] Create `ui_manager.h` / `ui_manager.cpp`
- [ ] Create `ui_component.h` with base class
- [ ] Implement `Button` component with event callbacks
- [ ] Implement `Slider` component
- [ ] Create `LayoutGrid` and `LayoutHelper` utilities
- [ ] Add overlap detection
- [ ] Update main loop to call `UIManager::processEvents()`

### Phase 2: Simple Mode Migration (Week 1-2)
Migrate simpler modes first to validate architecture:
- [ ] Keyboard mode (simple button grid)
- [ ] XY Pad mode (single TouchPad component)
- [ ] Bouncing Ball mode (few buttons)
- [ ] Physics Drop mode (few buttons)

### Phase 3: Complex Mode Migration (Week 2-3)
- [ ] Sequencer mode (Grid component)
- [ ] TB3PO mode (Grid + multiple controls)
- [ ] Grids mode (Grid + pattern controls)
- [ ] Euclidean mode (Grid + rhythm controls)

### Phase 4: Advanced Mode Migration (Week 3-4)
- [ ] Raga mode (10 buttons + slider + 4 control buttons)
- [ ] Morph mode (gesture recording + playback)
- [ ] Arpeggiator mode (complex controls)
- [ ] Auto Chord mode (key grid + controls)
- [ ] LFO mode (waveform + controls)
- [ ] Random Generator mode (multiple parameters)
- [ ] Grid Piano mode (large button grid)

### Phase 5: Testing & Validation (Week 4)
- [ ] Test all modes on 480×320 display (cyd35)
- [ ] Test all modes on 320×240 display (cyd28/cyd24)
- [ ] Verify no overlapping components
- [ ] Touch responsiveness testing
- [ ] Performance profiling
- [ ] Memory usage validation
- [ ] Documentation updates

---

## Coding Rules to Add

### Rule 1: No Direct Touch Handling in Modes
```
❌ FORBIDDEN:
void handleMyMode() {
  if (touch.justPressed && isButtonPressed(x, y, w, h)) {
    // ...
  }
}

✅ REQUIRED:
void initializeMyMode() {
  Button* btn = new Button(x, y, w, h, "Label");
  btn->onPress([]() {
    // Handle button press
  });
  UIManager::registerButton(btn);
}

void handleMyMode() {
  // NO TOUCH HANDLING - only animation/timing logic
  if (myState.playing) {
    updateAnimation();
  }
}
```

### Rule 2: All Layouts Must Be Calculated
```
❌ FORBIDDEN:
Button* btn = new Button(10, 100, 80, 45, "Button");

✅ REQUIRED:
int y = CONTENT_TOP + 20;
int btnW = (SCREEN_WIDTH - 40) / 3;
Button* btn = new Button(10, y, btnW, 45, "Button");

✅ BETTER:
LayoutGrid grid = UIManager::createGrid(1, 3, 10, y, SCREEN_WIDTH - 20, 45, 5);
Button* btn = new Button(grid.getCell(0, 0), "Button");
```

### Rule 3: No Overlapping Components
```
✅ REQUIRED in initializeMyMode():
if (UIManager::checkOverlaps()) {
  Serial.println("ERROR: Overlapping components in MyMode");
  // Fix layout before release
}
```

### Rule 4: UI Components Are Library Objects
```
❌ FORBIDDEN:
void drawMyButton(int x, int y) {
  tft.fillRoundRect(x, y, 80, 40, 8, THEME_PRIMARY);
  tft.drawString("Press", x + 40, y + 20, 2);
  if (touch.justPressed && isButtonPressed(x, y, 80, 40)) {
    handlePress();
  }
}

✅ REQUIRED:
Button* myButton = new Button(x, y, 80, 40, "Press", THEME_PRIMARY);
myButton->onPress([]() {
  handlePress();
});
UIManager::registerButton(myButton);
```

### Rule 5: Mode Files Structure
```cpp
// Required structure for all mode files:

// UI component pointers (module scope)
static Button* button1;
static Button* button2;
static Slider* slider1;

// Initialize - setup UI components
void initializeMyMode() {
  // 1. Reset mode state
  // 2. Clear previous UI: UIManager::clearMode()
  // 3. Create and register UI components
  // 4. Validate: UIManager::checkOverlaps()
  // 5. Draw static elements (non-interactive)
}

// Draw - render static/non-interactive elements only
void drawMyMode() {
  tft.fillScreen(THEME_BG);
  drawModuleHeader("MY MODE", true);
  // Draw text, graphics, non-interactive visuals
  // UI components draw themselves via UIManager
}

// Handle - animation and timing logic ONLY (no touch)
void handleMyMode() {
  // Update animations
  // Handle automatic playback
  // Update displays based on state changes
  // NO TOUCH HANDLING
}

// Event handlers - callbacks from UI components
void onButton1Press() {
  // Handle button 1
}

void onSlider1Change(float value) {
  // Handle slider change
}
```

---

## Success Criteria

### Functional
- ✅ All 15 modes work with event-driven UI
- ✅ Touch response is consistent across all modes
- ✅ No overlapping UI components
- ✅ Works correctly on 480×320 and 320×240 displays
- ✅ Button press feedback is immediate and consistent

### Code Quality
- ✅ Zero direct touch handling in mode files
- ✅ All layouts calculated (zero hardcoded coordinates)
- ✅ UI components are reusable library objects
- ✅ Mode files follow standard structure (init/draw/handle/callbacks)
- ✅ Overlap detection prevents layout bugs

### Performance
- ✅ Touch processing completes in <10ms per frame
- ✅ No increase in memory usage (component pooling if needed)
- ✅ No visual lag or stuttering

---

## Migration Checklist Per Mode

For each mode, complete these steps:

- [ ] **Analyze current touch handling**
  - Document all interactive areas (buttons, sliders, grids, pads)
  - Identify hardcoded coordinates
  - Note any overlapping issues

- [ ] **Design UI component layout**
  - Choose appropriate components (Button, Slider, Grid, TouchPad)
  - Calculate layout using LayoutGrid or LayoutHelper
  - Document z-ordering if components can overlap visually

- [ ] **Implement initialization**
  - Create UI components with calculated bounds
  - Register callbacks for each component
  - Register with UIManager
  - Validate no overlaps

- [ ] **Remove touch handling from handleMode()**
  - Delete all `if (touch.justPressed)` blocks
  - Move logic to callback functions
  - Keep only animation/timing logic

- [ ] **Test on both screen sizes**
  - Test on 480×320 (cyd35)
  - Test on 320×240 (cyd28)
  - Verify touch accuracy
  - Verify visual layout

- [ ] **Document changes**
  - Update mode documentation
  - Add code comments for complex layouts
  - Update DEV_NOTES.md if needed

---

## Files to Create/Modify

### New Files
- `src/ui_manager.h` - UIManager class declaration
- `src/ui_manager.cpp` - UIManager implementation
- `src/ui_component.h` - Base UIComponent class
- `src/ui_button.h` / `.cpp` - Button component
- `src/ui_slider.h` / `.cpp` - Slider component
- `src/ui_grid.h` / `.cpp` - Grid component
- `src/ui_touchpad.h` / `.cpp` - TouchPad component
- `src/layout_helper.h` / `.cpp` - Layout utilities

### Modified Files
- `src/CYD-MIDI-Controller.ino` - Add `UIManager::processEvents()` to loop
- `src/ui_elements.h` - Keep existing helpers, deprecate direct touch checks
- All 15 mode files (`.h` / `.cpp`) - Migrate to event-driven architecture
- `.github/copilot-instructions.md` - Add new coding rules
- `DEV_NOTES.md` - Update mode creation guide

---

## Testing Strategy

### Unit Testing
- Test each UI component in isolation
- Test layout calculations for both screen sizes
- Test overlap detection with various configurations
- Test event dispatch with mock touch events

### Integration Testing
- Test each mode after migration
- Test mode switching (ensure UIManager clears properly)
- Test touch accuracy across entire screen
- Test multi-touch scenarios (if applicable)

### Performance Testing
- Profile event processing time
- Monitor memory usage per mode
- Check for memory leaks (component cleanup)
- Test sustained interaction (button mashing)

### Regression Testing
- Verify all MIDI functionality still works
- Verify BLE connectivity still works
- Verify web server still works
- Verify SD card operations still work

---

## Risks & Mitigation

### Risk 1: Performance Overhead
**Impact:** Event system adds processing overhead  
**Mitigation:** 
- Profile early and often
- Use component pooling if needed
- Optimize hot paths (touch checking)

### Risk 2: Large Refactor Scope
**Impact:** 15 modes is a lot to change  
**Mitigation:**
- Phase approach (simple modes first)
- Keep old system working during migration
- Test each mode before moving to next

### Risk 3: Breaking Existing Functionality
**Impact:** MIDI, BLE, timing might break  
**Mitigation:**
- Don't touch MIDI/BLE code
- Keep timing logic separate from UI
- Regression test after each mode

### Risk 4: Screen Size Compatibility
**Impact:** Layouts might not work on 320×240  
**Mitigation:**
- Test on both sizes early
- Use scaling helpers consistently
- Validate minimum button sizes

---

## Future Enhancements (Post-Refactor)

Once event-driven architecture is in place:

1. **Touch Gestures:** Swipe, pinch, long-press
2. **Animations:** Smooth transitions, spring physics
3. **Themes:** Runtime theme switching
4. **UI Templates:** Common layouts (header+grid+footer)
5. **Accessibility:** Larger touch targets mode, high contrast
6. **Touch Calibration:** Improved auto-calibration
7. **Multi-Touch:** Support for simultaneous touches

---

## Questions to Resolve

1. **Memory Management:** Who owns UI components? (Proposal: UIManager owns, mode holds pointers)
2. **Draw Optimization:** Full redraw vs partial updates? (Proposal: Components handle own updates)
3. **Z-Ordering:** How to handle visual overlays? (Proposal: Registration order = draw order)
4. **Component Lifecycle:** When to delete old components? (Proposal: UIManager::clearMode() deletes all)
5. **Touch Debouncing:** How long? (Proposal: 50ms minimum between justPressed events)

---

## References

- Current touch handling: `src/ui_elements.h` (lines 92-153)
- Button feedback system: `src/ui_elements.h` Button class
- Example mode: `src/raga_mode.cpp` (current implementation)
- Layout constants: `src/common_definitions.h` (lines 41-76)
- Main loop: `src/CYD-MIDI-Controller.ino` (line 770+)

---

**Ready to proceed? Let's start with Phase 1!**
