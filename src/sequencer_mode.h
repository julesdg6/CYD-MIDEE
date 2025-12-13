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
  
  // Calculate control button layout from screen dimensions
  int btnY = SCREEN_HEIGHT - 60;
  int btnH = 45;
  int btnSpacing = 10;
  int btn1W = (SCREEN_WIDTH - (6 * btnSpacing)) / 5;
  
  // Initialize control buttons with calculated positioning
  seqBtnPlayStop.setBounds(btnSpacing, btnY, btn1W, btnH);
  seqBtnPlayStop.setText(sequencerPlaying ? "STOP" : "PLAY");
  seqBtnPlayStop.setColor(sequencerPlaying ? THEME_ERROR : THEME_SUCCESS);
  
  seqBtnClear.setBounds(btnSpacing * 2 + btn1W, btnY, btn1W, btnH);
  seqBtnClear.setText("CLEAR");
  seqBtnClear.setColor(THEME_WARNING);
  
  seqBtnBpmDown.setBounds(btnSpacing * 3 + btn1W * 2, btnY, btn1W, btnH);
  seqBtnBpmDown.setText("BPM-");
  seqBtnBpmDown.setColor(THEME_SECONDARY);
  
  seqBtnBpmUp.setBounds(btnSpacing * 4 + btn1W * 3, btnY, btn1W, btnH);
  seqBtnBpmUp.setText("BPM+");
  seqBtnBpmUp.setColor(THEME_SECONDARY);
  
  seqBtnMenu.setBounds(btnSpacing * 5 + btn1W * 4, btnY, btn1W, btnH);
  seqBtnMenu.setText("MENU");
  seqBtnMenu.setColor(THEME_PRIMARY);
  
  drawSequencerMode();
}

void drawSequencerMode() {
  tft.fillScreen(THEME_BG);
  
  // Use unified header with status icons
  drawModuleHeader("BEATS");
  
  drawSequencerGrid();
  
  // Calculate button layout from screen dimensions
  int btnY = SCREEN_HEIGHT - 60;
  int btnH = 45;
  int btnSpacing = 10;
  int btn1W = (SCREEN_WIDTH - (6 * btnSpacing)) / 5;
  
  // Transport controls - draw buttons with initial state
  seqBtnPlayStop.setText(sequencerPlaying ? "STOP" : "PLAY");
  seqBtnPlayStop.setColor(sequencerPlaying ? THEME_ERROR : THEME_SUCCESS);
  seqBtnPlayStop.draw(true);
  seqBtnClear.draw(true);
  seqBtnBpmDown.draw(true);
  seqBtnBpmUp.draw(true);
  seqBtnMenu.draw(true);
  
  // BPM display - positioned to the right of buttons
  tft.setTextColor(THEME_TEXT, THEME_BG);
  int bpmX = btnSpacing * 5 + btn1W * 4 + 20;
  if (midiClock.isReceiving) {
    tft.drawString(String((int)midiClock.calculatedBPM) + " [EXT]", bpmX, btnY + 15, 2);
  } else {
    tft.drawString(String((int)globalState.bpm), bpmX, btnY + 15, 2);
  }
}

void drawSequencerGrid() {
  // Calculate grid layout from screen dimensions
  int gridSpacing = 10;
  int gridX = gridSpacing;
  int gridY = CONTENT_TOP + 5;
  int availableWidth = SCREEN_WIDTH - (2 * gridSpacing);
  int availableHeight = SCREEN_HEIGHT - gridY - 80; // Leave space for controls
  
  // Calculate cell size to fill available space
  int labelWidth = 35;
  int cellSpacing = 2;
  int cellW = (availableWidth - labelWidth - (SEQ_STEPS + 1) * cellSpacing) / SEQ_STEPS;
  int cellH = (availableHeight - (SEQ_TRACKS + 1) * cellSpacing) / SEQ_TRACKS;
  
  // 808-style track labels and colors
  String trackLabels[] = {"KICK", "SNRE", "HHAT", "OPEN"};
  uint16_t trackColors[] = {THEME_ERROR, THEME_WARNING, THEME_PRIMARY, THEME_ACCENT};
  
  for (int track = 0; track < SEQ_TRACKS; track++) {
    int y = gridY + track * (cellH + cellSpacing);
    
    // Track name with color coding
    tft.setTextColor(trackColors[track], THEME_BG);
    tft.drawString(trackLabels[track], gridX, y + cellH/2 - 6, 1);
    
    // Steps - 16 steps in 808 style
    for (int step = 0; step < SEQ_STEPS; step++) {
      int x = gridX + labelWidth + step * (cellW + cellSpacing);
      
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
  if (touch.justPressed && isButtonPressed(BACK_BTN_X, BACK_BTN_Y, BTN_BACK_W, BTN_BACK_H)) {
    sequencerPlaying = false;
    exitToMenu();
    return;
  }
  
  // Calculate button layout from screen dimensions
  int btnY = SCREEN_HEIGHT - 60;
  int btnH = 45;
  int btnSpacing = 10;
  int btn1W = (SCREEN_WIDTH - (6 * btnSpacing)) / 5;
  
  // Update button visual states
  seqBtnPlayStop.draw();
  seqBtnClear.draw();
  seqBtnBpmDown.draw();
  seqBtnBpmUp.draw();
  seqBtnMenu.draw();
  
  // Handle touch input
  if (touch.justPressed) {
    // Transport controls
    if (isButtonPressed(btnSpacing, btnY, btn1W, btnH)) {
      sequencerPlaying = !sequencerPlaying;
      if (sequencerPlaying) {
        currentStep = 0;
        lastStepTime = millis();
      }
      drawSequencerMode();
      return;
    }
    
    if (isButtonPressed(btnSpacing * 2 + btn1W, btnY, btn1W, btnH)) {
      // Clear all patterns
      for (int t = 0; t < SEQ_TRACKS; t++) {
        for (int s = 0; s < SEQ_STEPS; s++) {
          sequencePattern[t][s] = false;
        }
      }
      drawSequencerGrid();
      return;
    }
    
    if (isButtonPressed(btnSpacing * 3 + btn1W * 2, btnY, btn1W, btnH)) {
      float newBpm = max(60.0f, globalState.bpm - 1.0f);
      setBPM(newBpm);
      stepInterval = 60000 / (int)globalState.bpm / 4;
      drawSequencerMode();
      return;
    }
    
    if (isButtonPressed(btnSpacing * 4 + btn1W * 3, btnY, btn1W, btnH)) {
      float newBpm = min(200.0f, globalState.bpm + 1.0f);
      setBPM(newBpm);
      stepInterval = 60000 / (int)globalState.bpm / 4;
      drawSequencerMode();
      return;
    }
    
    // Grid interaction - recalculate grid layout
    int gridSpacing = 10;
    int gridX = gridSpacing;
    int gridY = CONTENT_TOP + 5;
    int availableWidth = SCREEN_WIDTH - (2 * gridSpacing);
    int availableHeight = SCREEN_HEIGHT - gridY - 80;
    int labelWidth = 35;
    int cellSpacing = 2;
    int cellW = (availableWidth - labelWidth - (SEQ_STEPS + 1) * cellSpacing) / SEQ_STEPS;
    int cellH = (availableHeight - (SEQ_TRACKS + 1) * cellSpacing) / SEQ_TRACKS;
    
    for (int track = 0; track < SEQ_TRACKS; track++) {
      for (int step = 0; step < SEQ_STEPS; step++) {
        int x = gridX + labelWidth + step * (cellW + cellSpacing);
        int y = gridY + track * (cellH + cellSpacing);
        
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