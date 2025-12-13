#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "ui_component.h"
#include <functional>

class UIButton : public UIComponent {
public:
  using Callback = std::function<void()>;
  
  UIButton(int x, int y, int w, int h, String text, uint16_t color = THEME_PRIMARY);
  UIButton(const Rect& bounds, String text, uint16_t color = THEME_PRIMARY);
  
  // Event callbacks
  void onPress(Callback callback);
  void onRelease(Callback callback);
  
  // Customization
  void setText(String text);
  void setColor(uint16_t color);
  String getText() const { return text; }
  uint16_t getColor() const { return color; }
  
  // UIComponent interface
  void draw(bool force = false) override;
  bool checkEvent(const TouchState& touch) override;
  
private:
  String text;
  uint16_t color;
  Callback pressCallback;
  Callback releaseCallback;
  bool isPressed = false;
  bool wasPressed = false;
};

#endif // UI_BUTTON_H
