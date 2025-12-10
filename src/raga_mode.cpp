/*******************************************************************
 RAGA Mode Implementation
 Indian Classical Music scales with microtonal support
 *******************************************************************/

#include "raga_mode.h"

RagaState raga;

// Raga scale definitions
// notes[]: MIDI intervals from root (0=root, 1=minor 2nd, 2=major 2nd, etc.)
// microtonalCents[]: Fine tuning in cents (-50 to +50, 0 = no adjustment)
const RagaScale ragaScales[RAGA_COUNT] = {
  // Bhairavi - Morning raga, very devotional
  {"Bhairavi", {0, 1, 3, 5, 7, 8, 10, 12, 255, 255, 255, 255}, 8, 
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0xF800}, // Red
  
  // Lalit - Morning raga, complex and serious
  {"Lalit", {0, 1, 4, 6, 7, 9, 11, 12, 255, 255, 255, 255}, 8,
   {0, 0, 0, -20, 0, 0, 0, 0, 0, 0, 0, 0}, 0xFD00}, // Orange
  
  // Bhupali - Evening raga, pentatonic, peaceful
  {"Bhupali", {0, 2, 4, 7, 9, 12, 255, 255, 255, 255, 255, 255}, 6,
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0xFFE0}, // Yellow
  
  // Todi - Morning raga, intense and passionate
  {"Todi", {0, 1, 3, 6, 7, 8, 11, 12, 255, 255, 255, 255}, 8,
   {0, -30, 0, -20, 0, -20, 0, 0, 0, 0, 0, 0}, 0x07E0}, // Green
  
  // Madhuvanti - Evening raga, romantic
  {"Madhuvanti", {0, 2, 3, 6, 7, 9, 11, 12, 255, 255, 255, 255}, 8,
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0x07FF}, // Cyan
  
  // Meghmalhar - Monsoon raga, evokes rain
  {"Meghmalhar", {0, 2, 3, 5, 7, 9, 10, 12, 255, 255, 255, 255}, 8,
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0x001F}, // Blue
  
  // Yaman - Evening raga, very popular and soothing
  {"Yaman", {0, 2, 4, 6, 7, 9, 11, 12, 255, 255, 255, 255}, 8,
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0x781F}, // Purple
  
  // Kalavati - Night raga, tender and sweet
  {"Kalavati", {0, 2, 3, 5, 7, 9, 10, 12, 255, 255, 255, 255}, 8,
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0xF81F}, // Magenta
  
  // Malkauns - Late night raga, pentatonic, meditative
  {"Malkauns", {0, 3, 5, 8, 10, 12, 255, 255, 255, 255, 255, 255}, 6,
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0x8410}, // Dark gray
  
  // Bairagi - Morning raga, devotional
  {"Bairagi", {0, 1, 5, 7, 8, 12, 255, 255, 255, 255, 255, 255}, 6,
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0xFBE0}  // Light Orange
};

void initializeRagaMode() {
  Serial.println("\n=== Raga Mode Initialization ===");
  
  raga.currentRaga = BHAIRAVI;
  raga.rootNote = 60; // Middle C
  raga.playing = false;
  raga.droneEnabled = false;
  raga.tempo = 128; // Medium tempo
  raga.currentStep = 0;
  raga.lastNoteTime = 0;
  raga.currentNote = -1;
  raga.octaveRange = 2;
  
  Serial.printf("Raga: %s\n", ragaScales[raga.currentRaga].name);
  Serial.println("Raga mode initialized");
  
  drawRagaMode();
}

void drawRagaMode() {
  tft.fillScreen(THEME_BG);
  
  // Header
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawString("RAGA", 10, 10, 4);
  
  // Current raga name and status
  const RagaScale& current = ragaScales[raga.currentRaga];
  tft.setTextColor(current.color, THEME_BG);
  tft.drawString(current.name, 120, 15, 2);
  
  tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
  String status = raga.playing ? "PLAYING" : "STOPPED";
  tft.drawRightString(status, 470, 10, 2);
  
  int y = CONTENT_TOP;
  
  // Draw 10 raga selection buttons in 2 rows of 5
  int btnW = 90;
  int btnH = 50;
  int spacing = 6;
  int rowSpacing = 10;
  int startX = (480 - (5 * btnW + 4 * spacing)) / 2;
  
  for (int i = 0; i < RAGA_COUNT; i++) {
    int row = i / 5;
    int col = i % 5;
    int x = startX + col * (btnW + spacing);
    int btnY = y + row * (btnH + rowSpacing);
    
    uint16_t btnColor = (i == raga.currentRaga) ? ragaScales[i].color : THEME_SURFACE;
    uint16_t textColor = (i == raga.currentRaga) ? THEME_BG : THEME_TEXT;
    
    drawRoundButton(x, btnY, btnW, btnH, ragaScales[i].name, btnColor, textColor);
  }
  
  y += (2 * btnH) + rowSpacing + 20;
  
  // Scale visualization - show notes in current raga
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawString("Scale:", 20, y, 2);
  
  String scaleNotes = "";
  const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  for (int i = 0; i < current.numNotes; i++) {
    if (current.notes[i] != 255) {
      scaleNotes += noteNames[current.notes[i] % 12];
      if (current.microtonalCents[i] < 0) scaleNotes += "↓";
      else if (current.microtonalCents[i] > 0) scaleNotes += "↑";
      if (i < current.numNotes - 1) scaleNotes += " ";
    }
  }
  tft.setTextColor(current.color, THEME_BG);
  tft.drawString(scaleNotes, 80, y, 2);
  
  y += 30;
  
  // Control section
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawString("Root:", 20, y, 2);
  tft.drawString(noteNames[raga.rootNote % 12], 80, y, 2);
  
  tft.drawString("Drone:", 180, y, 2);
  tft.setTextColor(raga.droneEnabled ? THEME_SUCCESS : THEME_TEXT_DIM, THEME_BG);
  tft.drawString(raga.droneEnabled ? "ON" : "OFF", 260, y, 2);
  
  y += 30;
  
  // Tempo slider
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawString("Tempo:", 20, y, 2);
  int sliderX = 100;
  int sliderW = 300;
  int sliderH = 20;
  tft.drawRect(sliderX, y, sliderW, sliderH, THEME_TEXT);
  int tempoFill = (raga.tempo * sliderW) / 255;
  if (tempoFill > 0) {
    tft.fillRect(sliderX + 1, y + 1, tempoFill, sliderH - 2, current.color);
  }
  
  // Bottom control buttons
  int ctrlY = 260;
  int ctrlH = 50;
  int ctrlW = 80;
  
  drawRoundButton(10, ctrlY, ctrlW, ctrlH, raga.playing ? "STOP" : "PLAY", THEME_PRIMARY, THEME_TEXT);
  drawRoundButton(100, ctrlY, ctrlW, ctrlH, "DRONE", raga.droneEnabled ? THEME_SUCCESS : THEME_SECONDARY, THEME_TEXT);
  drawRoundButton(190, ctrlY, ctrlW, ctrlH, "ROOT-", THEME_ACCENT, THEME_TEXT);
  drawRoundButton(280, ctrlY, ctrlW, ctrlH, "ROOT+", THEME_ACCENT, THEME_TEXT);
  drawRoundButton(370, ctrlY, ctrlW, ctrlH, "<<", THEME_TEXT_DIM, THEME_TEXT);
}

void playRagaNote(uint8_t scaleIndex, bool slide) {
  const RagaScale& current = ragaScales[raga.currentRaga];
  
  if (scaleIndex >= current.numNotes || current.notes[scaleIndex] == 255) return;
  
  // Calculate MIDI note
  uint8_t note = raga.rootNote + current.notes[scaleIndex];
  
  // Apply microtonal adjustment using pitch bend
  int16_t cents = current.microtonalCents[scaleIndex];
  if (cents != 0) {
    // MIDI pitch bend: 8192 = center, ±8191 range
    // ±2 semitones = ±200 cents typical
    int16_t bendValue = 8192 + (cents * 8192 / 200);
    bendValue = constrain(bendValue, 0, 16383);
    
    // Send pitch bend (14-bit value, LSB first, MSB second)
    uint8_t lsb = bendValue & 0x7F;
    uint8_t msb = (bendValue >> 7) & 0x7F;
    sendMIDI(0xE0, lsb, msb); // Pitch bend
  }
  
  // If sliding, send gradual pitch bend from previous note
  if (slide && raga.currentNote >= 0) {
    // Quick slide effect
    for (int i = 0; i < 5; i++) {
      int16_t slideValue = 8192 + ((i - 2) * 400);
      slideValue = constrain(slideValue, 0, 16383);
      uint8_t lsb = slideValue & 0x7F;
      uint8_t msb = (slideValue >> 7) & 0x7F;
      sendMIDI(0xE0, lsb, msb);
      delay(10);
    }
  }
  
  // Stop previous note
  if (raga.currentNote >= 0) {
    sendMIDI(0x80, raga.currentNote, 0);
  }
  
  // Play new note
  sendMIDI(0x90, note, 100);
  raga.currentNote = note;
}

void startDrone() {
  // Play drone notes (root and fifth)
  sendMIDI(0x90, raga.rootNote, 60);
  sendMIDI(0x90, raga.rootNote + 7, 50); // Fifth
  sendMIDI(0x90, raga.rootNote + 12, 40); // Octave
}

void stopDrone() {
  sendMIDI(0x80, raga.rootNote, 0);
  sendMIDI(0x80, raga.rootNote + 7, 0);
  sendMIDI(0x80, raga.rootNote + 12, 0);
}

void handleRagaMode() {
  updateTouch();
  
  // Handle automatic phrase playback
  if (raga.playing) {
    unsigned long now = millis();
    unsigned long noteDelay = 100 + (raga.tempo * 4); // 100-1120ms per note
    
    if (now - raga.lastNoteTime >= noteDelay) {
      raga.lastNoteTime = now;
      
      const RagaScale& current = ragaScales[raga.currentRaga];
      
      // Simple ascending/descending pattern with occasional slides
      bool slide = (random(100) < 30); // 30% chance of slide
      playRagaNote(raga.currentStep, slide);
      
      // Move to next note in scale
      if (random(100) < 70) {
        raga.currentStep = (raga.currentStep + 1) % current.numNotes;
      } else {
        // Sometimes go backwards
        raga.currentStep = (raga.currentStep > 0) ? raga.currentStep - 1 : current.numNotes - 1;
      }
    }
  }
  
  if (touch.justPressed) {
    // Check raga selection buttons
    int btnW = 90;
    int btnH = 50;
    int spacing = 6;
    int rowSpacing = 10;
    int startX = (480 - (5 * btnW + 4 * spacing)) / 2;
    int y = CONTENT_TOP;
    
    for (int i = 0; i < RAGA_COUNT; i++) {
      int row = i / 5;
      int col = i % 5;
      int x = startX + col * (btnW + spacing);
      int btnY = y + row * (btnH + rowSpacing);
      
      if (isButtonPressed(x, btnY, btnW, btnH)) {
        raga.currentRaga = (RagaType)i;
        raga.currentStep = 0;
        if (raga.currentNote >= 0) {
          sendMIDI(0x80, raga.currentNote, 0);
          raga.currentNote = -1;
        }
        // Reset pitch bend
        sendMIDI(0xE0, 0, 64);
        drawRagaMode();
        Serial.printf("Selected raga: %s\n", ragaScales[i].name);
        return;
      }
    }
    
    // Check tempo slider
    int sliderY = CONTENT_TOP + 140;
    int sliderX = 100;
    int sliderW = 300;
    int sliderH = 20;
    
    if (touch.y >= sliderY && touch.y < sliderY + sliderH &&
        touch.x >= sliderX && touch.x < sliderX + sliderW) {
      raga.tempo = ((touch.x - sliderX) * 255) / sliderW;
      drawRagaMode();
      Serial.printf("Tempo: %d\n", raga.tempo);
      return;
    }
    
    // Control buttons
    int ctrlY = 260;
    int ctrlH = 50;
    int ctrlW = 80;
    
    // PLAY/STOP
    if (isButtonPressed(10, ctrlY, ctrlW, ctrlH)) {
      raga.playing = !raga.playing;
      if (raga.playing) {
        raga.currentStep = 0;
        raga.lastNoteTime = millis();
      } else {
        if (raga.currentNote >= 0) {
          sendMIDI(0x80, raga.currentNote, 0);
          raga.currentNote = -1;
        }
        sendMIDI(0xE0, 0, 64); // Reset pitch bend
      }
      drawRagaMode();
      Serial.printf("Raga %s\n", raga.playing ? "started" : "stopped");
      return;
    }
    
    // DRONE
    if (isButtonPressed(100, ctrlY, ctrlW, ctrlH)) {
      raga.droneEnabled = !raga.droneEnabled;
      if (raga.droneEnabled) {
        startDrone();
      } else {
        stopDrone();
      }
      drawRagaMode();
      Serial.printf("Drone %s\n", raga.droneEnabled ? "enabled" : "disabled");
      return;
    }
    
    // ROOT-
    if (isButtonPressed(190, ctrlY, ctrlW, ctrlH)) {
      if (raga.droneEnabled) stopDrone();
      raga.rootNote = constrain(raga.rootNote - 1, 36, 84);
      if (raga.droneEnabled) startDrone();
      drawRagaMode();
      Serial.printf("Root note: %d\n", raga.rootNote);
      return;
    }
    
    // ROOT+
    if (isButtonPressed(280, ctrlY, ctrlW, ctrlH)) {
      if (raga.droneEnabled) stopDrone();
      raga.rootNote = constrain(raga.rootNote + 1, 36, 84);
      if (raga.droneEnabled) startDrone();
      drawRagaMode();
      Serial.printf("Root note: %d\n", raga.rootNote);
      return;
    }
    
    // BACK
    if (isButtonPressed(370, ctrlY, ctrlW, ctrlH)) {
      if (raga.playing) {
        raga.playing = false;
        if (raga.currentNote >= 0) {
          sendMIDI(0x80, raga.currentNote, 0);
          raga.currentNote = -1;
        }
      }
      if (raga.droneEnabled) {
        stopDrone();
        raga.droneEnabled = false;
      }
      sendMIDI(0xE0, 0, 64); // Reset pitch bend
      exitToMenu();
      return;
    }
  }
}
