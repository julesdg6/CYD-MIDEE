#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "ui_component.h"
#include <functional>

class UISlider : public UIComponent {
public:
  using Callback = std::function<void(float value)>; // 0.0 - 1.0
  
  UISlider(int x, int y, int w, int h, float initialValue = 0.5f);
  UISlider(const Rect& bounds, float initialValue = 0.5f);
  
  // Event callback
  void onChange(Callback callback);
  
  // Value management
  void setValue(float value); // 0.0 - 1.0
  float getValue() const { return value; }
  
  // Display mapping (for label formatting)
  void setRange(float min, float max);
  float getDisplayValue() const { return displayMin + value * (displayMax - displayMin); }
  
  // Customization
  void setLabel(String label);
  void setColor(uint16_t color);
  String getLabel() const { return label; }
  
  // UIComponent interface
  void draw(bool force = false) override;
  bool checkEvent(const TouchState& touch) override;
  
private:
  String label;
  float value; // Always 0.0 - 1.0 internally
  float displayMin = 0.0f;
  float displayMax = 1.0f;
  uint16_t color = THEME_PRIMARY;
  Callback changeCallback;
  bool isTouched = false;
  float lastDrawnValue = -1.0f;
};

#endif // UI_SLIDER_H
