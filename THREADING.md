# Threading Architecture Migration

## Overview

This document describes the ongoing migration to a threaded architecture for touch input and MIDI I/O handling, with centralized BPM management.

## Goals

1. **Centralized Touch Handling** - Single touch input thread running on Core 0
2. **Centralized MIDI I/O** - Single MIDI thread running on Core 1 with message queue
3. **Global BPM** - Shared BPM value across all modules (no per-module BPM)
4. **Thread Safety** - Mutex-protected access to shared state

## Architecture

### Global State (`globalState`)

Located in `common_definitions.h`:

```cpp
struct GlobalState {
  float bpm = 120.0;              // Global BPM shared by all modules
  bool isPlaying = false;          // Global play/stop state
  unsigned long lastBeatTime = 0;  // Last beat timestamp
  int currentMidiChannel = 1;      // MIDI channel (1-16)
};
```

### Touch Thread (`TouchThread`)

**Purpose**: Handle all touchscreen input on dedicated thread

**Methods**:
- `begin()` - Initialize touch thread on Core 0
- `getState()` - Thread-safe access to current touch state
- `registerCallback()` - Register module-specific touch handler
- `unregisterCallback()` - Remove touch handler

**Implementation**: `src/thread_manager.cpp`

**Status**: ⚠️ **Partially implemented** - needs integration with calibration system

### MIDI Thread (`MIDIThread`)

**Purpose**: Handle all MIDI output on dedicated thread with message queue

**Methods**:
- `begin()` - Initialize MIDI thread on Core 1
- `sendNoteOn(note, velocity)` - Queue Note On message
- `sendNoteOff(note, velocity)` - Queue Note Off message  
- `sendCC(controller, value)` - Queue CC message
- `sendPitchBend(value)` - Queue Pitch Bend message
- `sendClock()` - Send MIDI clock tick
- `sendStart()` - Send MIDI start
- `sendStop()` - Send MIDI stop
- `setBPM(bpm)` - Update global BPM
- `getBPM()` - Get current BPM

**Implementation**: `src/thread_manager.cpp`

**Status**: ⚠️ **Partially implemented** - ready for module integration

## Migration Status

### Phase 1: Infrastructure ✅ COMPLETE

- [x] Define `GlobalState` struct
- [x] Create `TouchThread` class skeleton
- [x] Create `MIDIThread` class skeleton
- [x] Implement thread creation and mutex protection
- [x] Maintain backward compatibility with `MIDIClockSync`

### Phase 2: Touch Integration 🚧 IN PROGRESS

- [ ] Integrate touch calibration with thread
- [ ] Update `updateTouch()` to use threaded input
- [ ] Add external touch state access declaration
- [ ] Test touch accuracy with threading

### Phase 3: MIDI Integration 📋 PLANNED

- [ ] Replace direct MIDI calls with `MIDIThread::sendNoteOn()` etc.
- [ ] Update sequencer modes to use global BPM
- [ ] Update LFO/arpeggiator to reference `globalState.bpm`
- [ ] Remove per-module BPM variables

### Phase 4: Module Refactoring 📋 PLANNED

Each mode needs to:
1. Use `globalState.bpm` instead of local BPM
2. Call `MIDIThread::sendNoteOn/Off()` instead of direct MIDI
3. Register touch callback via `TouchThread::registerCallback()`
4. Remove redundant touch/MIDI handling code

**Modules to update**:
- [ ] keyboard_mode.h
- [ ] sequencer_mode.h  
- [ ] bouncing_ball_mode.h
- [ ] physics_drop_mode.h
- [ ] random_generator_mode.h
- [ ] xy_pad_mode.h
- [ ] arpeggiator_mode.h
- [ ] grid_piano_mode.h
- [ ] auto_chord_mode.h
- [ ] lfo_mode.h
- [ ] tb3po_mode.h
- [ ] grids_mode.h
- [ ] raga_mode.h
- [ ] euclidean_mode.h
- [ ] morph_mode.h

## Compatibility Layer

To maintain backward compatibility during migration, we keep:

```cpp
struct MIDIClockSync {
  float calculatedBPM;  // Synced with globalState.bpm
  bool isPlaying;       // Synced with globalState.isPlaying
  // ... other fields
};
```

The main loop syncs these values:

```cpp
if (midiClock.isReceiving) {
  globalState.bpm = midiClock.calculatedBPM;
  globalState.isPlaying = midiClock.isPlaying;
}
```

## Benefits

1. **Consistency** - All modules use same BPM/touch/MIDI system
2. **Performance** - Dedicated threads prevent blocking
3. **Maintainability** - Centralized logic easier to debug
4. **Scalability** - Easy to add new modules without duplicating code
5. **Thread Safety** - Mutex protection prevents race conditions

## Testing Plan

1. Verify compilation with threading infrastructure
2. Test touch accuracy with/without threading
3. Test MIDI timing with queued messages
4. Verify BPM sync across modes
5. Stress test with rapid mode switching

## Known Issues

- TouchThread needs external `ts` object access
- Touch calibration currently in main thread
- MIDI thread needs BLE connection check
- Clock generation needs BPM sync testing

## Future Enhancements

- Separate display rendering thread
- SD card I/O thread for non-blocking writes
- Web server on dedicated thread
- MIDI input handling (currently only output)
- Touch gesture recognition in thread

## References

- `src/common_definitions.h` - Core definitions
- `src/thread_manager.cpp` - Thread implementations
- `src/ui_elements.h` - Touch utilities
- `src/midi_utils.h` - MIDI utilities

---

**Last Updated**: 2025-12-10
**Status**: Infrastructure complete, integration pending
