#include "ui_slider.h"

UISlider::UISlider(int x, int y, int w, int h, float initialValue)
  : UIComponent(x, y, w, h), value(constrain(initialValue, 0.0f, 1.0f)) {
}

UISlider::UISlider(const Rect& bounds, float initialValue)
  : UIComponent(bounds), value(constrain(initialValue, 0.0f, 1.0f)) {
}

void UISlider::onChange(Callback callback) {
  changeCallback = callback;
}

void UISlider::setValue(float newValue) {
  newValue = constrain(newValue, 0.0f, 1.0f);
  if (abs(value - newValue) > 0.001f) { // Avoid floating point noise
    value = newValue;
    draw(true);
  }
}

void UISlider::setRange(float min, float max) {
  displayMin = min;
  displayMax = max;
}

void UISlider::setLabel(String newLabel) {
  label = newLabel;
  draw(true);
}

void UISlider::setColor(uint16_t newColor) {
  if (color != newColor) {
    color = newColor;
    draw(true);
  }
}

void UISlider::draw(bool force) {
  if (!visible) return;
  
  // Only redraw if value changed or forced
  if (force || abs(value - lastDrawnValue) > 0.001f) {
    // Draw slider background
    tft.drawRect(bounds.x, bounds.y, bounds.w, bounds.h, THEME_TEXT);
    
    // Draw filled portion
    int fillWidth = (int)(value * (bounds.w - 2));
    if (fillWidth > 0) {
      tft.fillRect(bounds.x + 1, bounds.y + 1, fillWidth, bounds.h - 2, color);
    }
    
    // Clear unfilled portion
    if (fillWidth < bounds.w - 2) {
      tft.fillRect(bounds.x + 1 + fillWidth, bounds.y + 1, 
                   bounds.w - 2 - fillWidth, bounds.h - 2, THEME_BG);
    }
    
    lastDrawnValue = value;
  }
}

bool UISlider::checkEvent(const TouchState& touchState) {
  if (!enabled || !visible) return false;
  
  bool wasTouched = isTouched;
  isTouched = touchState.isPressed && contains(touchState.x, touchState.y);
  
  if (isTouched) {
    // Calculate value from touch position
    float newValue = (float)(touchState.x - bounds.x) / (float)bounds.w;
    newValue = constrain(newValue, 0.0f, 1.0f);
    
    if (abs(value - newValue) > 0.01f) { // Threshold to avoid noise
      value = newValue;
      draw();
      
      if (changeCallback) {
        changeCallback(value);
      }
    }
    return true;
  }
  
  return false;
}
