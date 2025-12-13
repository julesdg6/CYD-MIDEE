#ifndef MIDI_UTILS_H
#define MIDI_UTILS_H

#include "common_definitions.h"
#include "ui_elements.h"  // For Button class

// External variables
extern uint8_t midiChannel;
extern bool bleEnabled;

// Button objects from modes (for cleanup in stopAllModes)
extern Button seqBtnPlayStop, seqBtnClear, seqBtnBpmDown, seqBtnBpmUp, seqBtnMenu;
extern Button keyboardBtnOctDown, keyboardBtnOctUp, keyboardBtnScale, keyboardBtnKeyDown, keyboardBtnKeyUp, keyboardBtnMenu;
extern Button xyBtnXccDown, xyBtnXccUp, xyBtnYccDown, xyBtnYccUp;

// Scale definitions (implementations in midi_utils.cpp)
extern const Scale scales[];
extern const int NUM_SCALES;

// Legacy MIDI utility functions (kept for backward compatibility)
inline void sendMIDI(byte cmd, byte note, byte vel) {
  if (!globalState.bleConnected) return;
  
  // Apply MIDI channel (channels 1-16 are encoded as 0-15 in the lower nibble)
  byte channelCmd = (cmd & 0xF0) | ((midiChannel - 1) & 0x0F);
  
  midiPacket[2] = channelCmd;
  midiPacket[3] = note;
  midiPacket[4] = vel;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
}

// Threaded MIDI functions (preferred - use these for new code)
inline void sendNoteOn(uint8_t note, uint8_t velocity = 127) {
  MIDIThread::sendNoteOn(note, velocity);
}

inline void sendNoteOff(uint8_t note, uint8_t velocity = 0) {
  MIDIThread::sendNoteOff(note, velocity);
}

inline void sendControlChange(uint8_t controller, uint8_t value) {
  MIDIThread::sendCC(controller, value);
}

inline void sendPitchBend(int16_t value) {
  MIDIThread::sendPitchBend(value);
}

inline void setBPM(float bpm) {
  MIDIThread::setBPM(bpm);
  globalState.bpm = bpm;
  midiClock.calculatedBPM = bpm;  // Sync legacy struct
}

inline float getBPM() {
  return MIDIThread::getBPM();
}

inline void stopAllModes() {
  // Stop all MIDI notes using threaded system
  for (int i = 0; i < 128; i++) {
    MIDIThread::sendNoteOff(i, 0);
  }
  
  // Clear Button objects to prevent drawing on other screens
  // (Button class from ui_elements.h has persistent bounds that must be cleared)
  seqBtnPlayStop.setBounds(0, 0, 0, 0);
  seqBtnClear.setBounds(0, 0, 0, 0);
  seqBtnBpmDown.setBounds(0, 0, 0, 0);
  seqBtnBpmUp.setBounds(0, 0, 0, 0);
  seqBtnMenu.setBounds(0, 0, 0, 0);
  
  keyboardBtnOctDown.setBounds(0, 0, 0, 0);
  keyboardBtnOctUp.setBounds(0, 0, 0, 0);
  keyboardBtnScale.setBounds(0, 0, 0, 0);
  keyboardBtnKeyDown.setBounds(0, 0, 0, 0);
  keyboardBtnKeyUp.setBounds(0, 0, 0, 0);
  keyboardBtnMenu.setBounds(0, 0, 0, 0);
  
  xyBtnXccDown.setBounds(0, 0, 0, 0);
  xyBtnXccUp.setBounds(0, 0, 0, 0);
  xyBtnYccDown.setBounds(0, 0, 0, 0);
  xyBtnYccUp.setBounds(0, 0, 0, 0);
}

// Function declarations (implementations in midi_utils.cpp)
int getNoteInScale(int scaleIndex, int degree, int octave);
String getNoteNameFromMIDI(int midiNote);

#endif