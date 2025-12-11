#ifndef SEQUENCER_MODE_H
#define SEQUENCER_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Sequencer mode variables
#define SEQ_STEPS 16
#define SEQ_TRACKS 4
bool sequencePattern[SEQ_TRACKS][SEQ_STEPS];
int currentStep = 0;
unsigned long lastStepTime = 0;
unsigned long noteOffTime[SEQ_TRACKS] = {0};
int stepInterval;
bool sequencerPlaying = false;

// Control buttons
Button seqBtnPlayStop;
Button seqBtnClear;
Button seqBtnBpmDown;
Button seqBtnBpmUp;
Button seqBtnMenu;

// Function declarations
void initializeSequencerMode();
void drawSequencerMode();
void handleSequencerMode();
void drawSequencerGrid();
void toggleSequencerStep(int track, int step);
void updateSequencer();
void playSequencerStep();

// Implementations
void initializeSequencerMode() {
  stepInterval = 60000 / (int)globalState.bpm / 4; // 16th notes
  sequencerPlaying = false;
  currentStep = 0;
  
  // Clear all patterns
  for (int t = 0; t < SEQ_TRACKS; t++) {
    for (int s = 0; s < SEQ_STEPS; s++) {
      sequencePattern[t][s] = false;
    }
  }
  
  // Initialize control buttons
  int btnY = 245;
  int btnH = 45;
  seqBtnPlayStop.setBounds(10, btnY, 70, btnH);
  seqBtnPlayStop.setText(sequencerPlaying ? "STOP" : "PLAY");
  seqBtnPlayStop.setColor(sequencerPlaying ? THEME_ERROR : THEME_SUCCESS);
  
  seqBtnClear.setBounds(90, btnY, 70, btnH);
  seqBtnClear.setText("CLEAR");
  seqBtnClear.setColor(THEME_WARNING);
  
  seqBtnBpmDown.setBounds(170, btnY, 60, btnH);
  seqBtnBpmDown.setText("BPM-");
  seqBtnBpmDown.setColor(THEME_SECONDARY);
  
  seqBtnBpmUp.setBounds(240, btnY, 60, btnH);
  seqBtnBpmUp.setText("BPM+");
  seqBtnBpmUp.setColor(THEME_SECONDARY);
  
  seqBtnMenu.setBounds(380, btnY, 90, btnH);
  seqBtnMenu.setText("MENU");
  seqBtnMenu.setColor(THEME_PRIMARY);
}

void drawSequencerMode() {
  tft.fillScreen(THEME_BG);
  
  // Use unified header with status icons
  drawModuleHeader("BEATS");
  
  drawSequencerGrid();
  
  // Transport controls - draw buttons with initial state
  seqBtnPlayStop.setText(sequencerPlaying ? "STOP" : "PLAY");
  seqBtnPlayStop.setColor(sequencerPlaying ? THEME_ERROR : THEME_SUCCESS);
  seqBtnPlayStop.draw(true);
  seqBtnClear.draw(true);
  seqBtnBpmDown.draw(true);
  seqBtnBpmUp.draw(true);
  seqBtnMenu.draw(true);
  
  // BPM display
  tft.setTextColor(THEME_TEXT, THEME_BG);
  if (midiClock.isReceiving) {
    tft.drawString(String((int)midiClock.calculatedBPM) + " [EXT]", 310, btnY + 15, 2);
  } else {
    tft.drawString(String((int)globalState.bpm), 310, btnY + 15, 2);
  }
}

void drawSequencerGrid() {
  int gridX = 10;
  int gridY = 55;
  int cellW = 27;  // Increased from 15 for easier tapping
  int cellH = 42;  // Increased from 28 for easier tapping
  int spacing = 2;  // Slightly more spacing
  
  // 808-style track labels and colors
  String trackLabels[] = {"KICK", "SNRE", "HHAT", "OPEN"};
  uint16_t trackColors[] = {THEME_ERROR, THEME_WARNING, THEME_PRIMARY, THEME_ACCENT};
  
  for (int track = 0; track < SEQ_TRACKS; track++) {
    int y = gridY + track * (cellH + spacing + 3);
    
    // Track name with color coding
    tft.setTextColor(trackColors[track], THEME_BG);
    tft.drawString(trackLabels[track], gridX, y + 12, 1);
    
    // Steps - 16 steps in 808 style
    for (int step = 0; step < SEQ_STEPS; step++) {
      int x = gridX + 35 + step * (cellW + spacing);
      
      bool active = sequencePattern[track][step];
      bool current = (sequencerPlaying && step == currentStep);
      
      uint16_t color;
      if (current && active) color = THEME_TEXT;
      else if (current) color = trackColors[track];
      else if (active) color = trackColors[track];
      else color = THEME_SURFACE;
      
      // Highlight every 4th step (like 808)
      if (step % 4 == 0) {
        tft.drawRect(x-1, y-1, cellW+2, cellH+2, THEME_TEXT_DIM);
      }
      
      tft.fillRect(x, y, cellW, cellH, color);
      tft.drawRect(x, y, cellW, cellH, THEME_TEXT_DIM);
    }
  }
}

void handleSequencerMode() {
  // Back button - larger touch area
  if (touch.justPressed && isButtonPressed(10, 5, 70, 35)) {
    sequencerPlaying = false;
    exitToMenu();
    return;
  }
  
  // Update button visual states
  seqBtnPlayStop.draw();
  seqBtnClear.draw();
  seqBtnBpmDown.draw();
  seqBtnBpmUp.draw();
  seqBtnMenu.draw();
  
  // Handle touch input
  if (touch.justPressed) {
    // Transport controls
    if (seqBtnPlayStop.justPressed()) {
      sequencerPlaying = !sequencerPlaying;
      if (sequencerPlaying) {
        currentStep = 0;
        lastStepTime = millis();
      }
      drawSequencerMode();
      return;
    }
    
    if (seqBtnClear.justPressed()) {
      // Clear all patterns
      for (int t = 0; t < SEQ_TRACKS; t++) {
        for (int s = 0; s < SEQ_STEPS; s++) {
          sequencePattern[t][s] = false;
        }
      }
      drawSequencerGrid();
      return;
    }
    
    if (seqBtnBpmDown.justPressed()) {
      float newBpm = max(60.0f, globalState.bpm - 1.0f);
      setBPM(newBpm);
      stepInterval = 60000 / (int)globalState.bpm / 4;
      drawSequencerMode();
      return;
    }
    
    if (seqBtnBpmUp.justPressed()) {
      float newBpm = min(200.0f, globalState.bpm + 1.0f);
      setBPM(newBpm);
      stepInterval = 60000 / (int)globalState.bpm / 4;
      drawSequencerMode();
      return;
    }
    
    // Grid interaction
    int gridX = 45;
    int gridY = 55;
    int cellW = 27;
    int cellH = 42;
    int spacing = 2;
    
    for (int track = 0; track < SEQ_TRACKS; track++) {
      for (int step = 0; step < SEQ_STEPS; step++) {
        int x = gridX + step * (cellW + spacing);
        int y = gridY + track * (cellH + spacing + 3);
        
        if (isButtonPressed(x, y, cellW, cellH)) {
          toggleSequencerStep(track, step);
          drawSequencerGrid();
          return;
        }
      }
    }
  }
  
  // Update sequencer timing
  updateSequencer();
}

void toggleSequencerStep(int track, int step) {
  sequencePattern[track][step] = !sequencePattern[track][step];
}

void updateSequencer() {
  if (!sequencerPlaying) return;
  
  unsigned long now = millis();
  
  // Check for notes to turn off
  int drumNotes[] = {36, 38, 42, 46};
  for (int track = 0; track < SEQ_TRACKS; track++) {
    if (noteOffTime[track] > 0 && now >= noteOffTime[track]) {
      sendNoteOff(drumNotes[track]);
      noteOffTime[track] = 0;
    }
  }
  
  // Use MIDI clock if available, otherwise use internal timing
  unsigned long effectiveInterval;
  if (midiClock.isReceiving && midiClock.clockInterval > 0) {
    // MIDI clock is 24 ppqn, we want 16th notes (4 per quarter note)
    // So 6 clock pulses per 16th note
    effectiveInterval = midiClock.clockInterval * 6;
    
    // Auto-start on MIDI start message
    if (midiClock.isPlaying && !sequencerPlaying) {
      sequencerPlaying = true;
      currentStep = 0;
      lastStepTime = now;
    }
  } else {
    effectiveInterval = stepInterval;
  }
  
  if (now - lastStepTime >= effectiveInterval) {
    playSequencerStep();
    currentStep = (currentStep + 1) % SEQ_STEPS;
    lastStepTime = now;
    drawSequencerGrid();
  }
}

void playSequencerStep() {
  if (!globalState.bleConnected) return;
  
  int drumNotes[] = {36, 38, 42, 46}; // Kick, Snare, Hi-hat, Open Hi-hat
  int noteLengths[] = {200, 150, 50, 300}; // Note lengths in ms
  
  unsigned long now = millis();
  
  for (int track = 0; track < SEQ_TRACKS; track++) {
    if (sequencePattern[track][currentStep]) {
      // Turn on note
      sendNoteOn(drumNotes[track], 100);
      // Schedule note off
      noteOffTime[track] = now + noteLengths[track];
    }
  }
}

#endif