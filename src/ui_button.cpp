#include "ui_button.h"
#include "ui_elements.h" // For drawRoundButton

UIButton::UIButton(int x, int y, int w, int h, String text, uint16_t color)
  : UIComponent(x, y, w, h), text(text), color(color) {
}

UIButton::UIButton(const Rect& bounds, String text, uint16_t color)
  : UIComponent(bounds), text(text), color(color) {
}

void UIButton::onPress(Callback callback) {
  pressCallback = callback;
}

void UIButton::onRelease(Callback callback) {
  releaseCallback = callback;
}

void UIButton::setText(String newText) {
  if (text != newText) {
    text = newText;
    draw(true); // Force redraw with new text
  }
}

void UIButton::setColor(uint16_t newColor) {
  if (color != newColor) {
    color = newColor;
    draw(true); // Force redraw with new color
  }
}

void UIButton::draw(bool force) {
  if (!visible) return;
  
  // Only redraw if state changed or forced
  if (force || isPressed != lastDrawnPressed) {
    drawRoundButton(bounds.x, bounds.y, bounds.w, bounds.h, text, color, isPressed);
    lastDrawnPressed = isPressed;
  }
}

bool UIButton::checkEvent(const TouchState& touch) {
  if (!enabled || !visible) return false;
  
  wasPressed = isPressed;
  isPressed = touch.isPressed && contains(touch.x, touch.y);
  
  // Trigger press callback on press down
  if (isPressed && !wasPressed && pressCallback) {
    pressCallback();
    draw(); // Update visual feedback
    return true;
  }
  
  // Trigger release callback on release
  if (!isPressed && wasPressed && releaseCallback) {
    releaseCallback();
    draw(); // Update visual feedback
    return true;
  }
  
  // Update visual if press state changed
  if (isPressed != wasPressed) {
    draw();
  }
  
  return isPressed;
}
