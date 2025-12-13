#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "ui_component.h"
#include "ui_button.h"
#include "ui_slider.h"
#include <vector>

class UIManager {
public:
  // Lifecycle
  static void init();
  static void clearMode(); // Clear all components for mode switch
  
  // Component registration
  static void registerComponent(UIComponent* component);
  static void registerButton(UIButton* button);
  static void registerSlider(UISlider* slider);
  
  // Event processing (called from main loop after updateTouch())
  static void processEvents();
  
  // Drawing (call after mode initialization)
  static void drawAll(bool force = false);
  
  // Validation
  static bool checkOverlaps(); // Returns true if overlaps detected
  static void debugDrawBounds(); // Visual debug mode
  
  // Access to current components
  static const std::vector<UIComponent*>& getComponents() { return components; }
  
private:
  static std::vector<UIComponent*> components;
  static TouchState lastProcessedTouch;
  static bool debugMode;
};

#endif // UI_MANAGER_H
