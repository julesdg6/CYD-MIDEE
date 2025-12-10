#include "common_definitions.h"
#include <Arduino.h>

// Global state instance
GlobalState globalState;

// TouchThread implementation
TouchCallback TouchThread::activeCallback = nullptr;
TouchState TouchThread::currentState;
SemaphoreHandle_t TouchThread::touchMutex = nullptr;

void TouchThread::begin() {
  touchMutex = xSemaphoreCreateMutex();
  currentState.wasPressed = false;
  currentState.isPressed = false;
  currentState.justPressed = false;
  currentState.justReleased = false;
  currentState.x = 0;
  currentState.y = 0;
  
  // Create touch handling task on Core 0
  xTaskCreatePinnedToCore(
    touchTask,
    "TouchTask",
    4096,
    nullptr,
    2,  // Priority
    nullptr,
    0   // Core 0
  );
}

void TouchThread::update() {
  // Main loop update - handled by task now
}

void TouchThread::registerCallback(TouchCallback callback) {
  if (xSemaphoreTake(touchMutex, portMAX_DELAY)) {
    activeCallback = callback;
    xSemaphoreGive(touchMutex);
  }
}

void TouchThread::unregisterCallback() {
  if (xSemaphoreTake(touchMutex, portMAX_DELAY)) {
    activeCallback = nullptr;
    xSemaphoreGive(touchMutex);
  }
}

TouchState TouchThread::getState() {
  TouchState state;
  if (xSemaphoreTake(touchMutex, portMAX_DELAY)) {
    state = currentState;
    xSemaphoreGive(touchMutex);
  }
  return state;
}

void TouchThread::touchTask(void* parameter) {
  TS_Point rawPoint;
  
  while (true) {
    if (xSemaphoreTake(touchMutex, portMAX_DELAY)) {
      // Update touch state
      currentState.wasPressed = currentState.isPressed;
      
      if (ts.touched()) {
        rawPoint = ts.getPoint();
        
        // Apply calibration
        int x = map(rawPoint.x, 3700, 500, 0, SCREEN_WIDTH);
        int y = map(rawPoint.y, 400, 3700, 0, SCREEN_HEIGHT);
        
        x = constrain(x, 0, SCREEN_WIDTH - 1);
        y = constrain(y, 0, SCREEN_HEIGHT - 1);
        
        currentState.x = x;
        currentState.y = y;
        currentState.isPressed = true;
        currentState.justPressed = !currentState.wasPressed;
        currentState.justReleased = false;
        
        // Call active callback if registered
        if (activeCallback != nullptr) {
          activeCallback(x, y, true);
        }
      } else {
        if (currentState.wasPressed) {
          currentState.justReleased = true;
          if (activeCallback != nullptr) {
            activeCallback(currentState.x, currentState.y, false);
          }
        } else {
          currentState.justReleased = false;
        }
        currentState.isPressed = false;
        currentState.justPressed = false;
      }
      
      xSemaphoreGive(touchMutex);
    }
    
    vTaskDelay(10 / portTICK_PERIOD_MS);  // 100Hz polling
  }
}

// MIDIThread implementation
QueueHandle_t MIDIThread::midiQueue = nullptr;
SemaphoreHandle_t MIDIThread::midiMutex = nullptr;

void MIDIThread::begin() {
  midiMutex = xSemaphoreCreateMutex();
  midiQueue = xQueueCreate(64, sizeof(MIDIMessage));
  
  // Create MIDI handling task on Core 1
  xTaskCreatePinnedToCore(
    midiTask,
    "MIDITask",
    4096,
    nullptr,
    1,  // Priority
    nullptr,
    1   // Core 1
  );
}

void MIDIThread::sendNoteOn(uint8_t note, uint8_t velocity) {
  MIDIMessage msg;
  msg.type = MIDIMessage::NOTE_ON;
  msg.data1 = note;
  msg.data2 = velocity;
  xQueueSend(midiQueue, &msg, 0);
}

void MIDIThread::sendNoteOff(uint8_t note, uint8_t velocity) {
  MIDIMessage msg;
  msg.type = MIDIMessage::NOTE_OFF;
  msg.data1 = note;
  msg.data2 = velocity;
  xQueueSend(midiQueue, &msg, 0);
}

void MIDIThread::sendCC(uint8_t controller, uint8_t value) {
  MIDIMessage msg;
  msg.type = MIDIMessage::CC;
  msg.data1 = controller;
  msg.data2 = value;
  xQueueSend(midiQueue, &msg, 0);
}

void MIDIThread::sendPitchBend(int16_t value) {
  MIDIMessage msg;
  msg.type = MIDIMessage::PITCH_BEND;
  msg.data16 = value;
  xQueueSend(midiQueue, &msg, 0);
}

void MIDIThread::sendClock() {
  MIDIMessage msg;
  msg.type = MIDIMessage::CLOCK;
  xQueueSend(midiQueue, &msg, 0);
}

void MIDIThread::sendStart() {
  MIDIMessage msg;
  msg.type = MIDIMessage::START;
  xQueueSend(midiQueue, &msg, 0);
}

void MIDIThread::sendStop() {
  MIDIMessage msg;
  msg.type = MIDIMessage::STOP;
  xQueueSend(midiQueue, &msg, 0);
}

void MIDIThread::setBPM(float bpm) {
  if (xSemaphoreTake(midiMutex, portMAX_DELAY)) {
    globalState.bpm = constrain(bpm, 20.0, 300.0);
    xSemaphoreGive(midiMutex);
  }
}

float MIDIThread::getBPM() {
  float bpm = 120.0;
  if (xSemaphoreTake(midiMutex, portMAX_DELAY)) {
    bpm = globalState.bpm;
    xSemaphoreGive(midiMutex);
  }
  return bpm;
}

void MIDIThread::midiTask(void* parameter) {
  MIDIMessage msg;
  unsigned long lastClockTime = 0;
  unsigned long clockInterval = 0;
  
  while (true) {
    // Calculate clock interval from BPM
    if (xSemaphoreTake(midiMutex, 1)) {
      clockInterval = (unsigned long)((60000.0 / globalState.bpm) / 24.0);  // 24 PPQN
      xSemaphoreGive(midiMutex);
    }
    
    // Send MIDI clock if playing
    if (globalState.isPlaying) {
      unsigned long now = millis();
      if (now - lastClockTime >= clockInterval) {
        sendClock();
        lastClockTime = now;
      }
    }
    
    // Process queued MIDI messages
    if (xQueueReceive(midiQueue, &msg, 1 / portTICK_PERIOD_MS)) {
      if (!deviceConnected) {
        continue;  // Skip if no BLE connection
      }
      
      uint8_t channel = globalState.currentMidiChannel - 1;  // 0-15
      
      switch (msg.type) {
        case MIDIMessage::NOTE_ON:
          midiPacket[2] = 0x90 | channel;  // Note On + channel
          midiPacket[3] = msg.data1;       // Note
          midiPacket[4] = msg.data2;       // Velocity
          pCharacteristic->setValue(midiPacket, 5);
          pCharacteristic->notify();
          break;
          
        case MIDIMessage::NOTE_OFF:
          midiPacket[2] = 0x80 | channel;  // Note Off + channel
          midiPacket[3] = msg.data1;       // Note
          midiPacket[4] = msg.data2;       // Velocity
          pCharacteristic->setValue(midiPacket, 5);
          pCharacteristic->notify();
          break;
          
        case MIDIMessage::CC:
          midiPacket[2] = 0xB0 | channel;  // CC + channel
          midiPacket[3] = msg.data1;       // Controller
          midiPacket[4] = msg.data2;       // Value
          pCharacteristic->setValue(midiPacket, 5);
          pCharacteristic->notify();
          break;
          
        case MIDIMessage::PITCH_BEND:
          {
            uint16_t bend = msg.data16 + 8192;  // Center at 8192
            midiPacket[2] = 0xE0 | channel;     // Pitch Bend + channel
            midiPacket[3] = bend & 0x7F;        // LSB
            midiPacket[4] = (bend >> 7) & 0x7F; // MSB
            pCharacteristic->setValue(midiPacket, 5);
            pCharacteristic->notify();
          }
          break;
          
        case MIDIMessage::CLOCK:
          midiPacket[2] = 0xF8;  // MIDI Clock
          pCharacteristic->setValue(midiPacket, 3);
          pCharacteristic->notify();
          break;
          
        case MIDIMessage::START:
          midiPacket[2] = 0xFA;  // MIDI Start
          pCharacteristic->setValue(midiPacket, 3);
          pCharacteristic->notify();
          globalState.isPlaying = true;
          break;
          
        case MIDIMessage::STOP:
          midiPacket[2] = 0xFC;  // MIDI Stop
          pCharacteristic->setValue(midiPacket, 3);
          pCharacteristic->notify();
          globalState.isPlaying = false;
          break;
      }
    }
    
    vTaskDelay(1 / portTICK_PERIOD_MS);  // 1ms tick
  }
}
