#include "ui_manager.h"

// Static member initialization
std::vector<UIComponent*> UIManager::components;
TouchState UIManager::lastProcessedTouch;
bool UIManager::debugMode = false;

void UIManager::init() {
  components.clear();
  lastProcessedTouch = TouchState();
  debugMode = false;
  Serial.println("[UIManager] Initialized");
}

void UIManager::clearMode() {
  // Delete all registered components
  for (auto* component : components) {
    delete component;
  }
  components.clear();
  Serial.println("[UIManager] Cleared all components");
}

void UIManager::registerComponent(UIComponent* component) {
  if (component) {
    components.push_back(component);
  }
}

void UIManager::registerButton(UIButton* button) {
  registerComponent(button);
}

void UIManager::registerSlider(UISlider* slider) {
  registerComponent(slider);
}

void UIManager::processEvents() {
  extern TouchState touch; // Access global touch state
  
  // Process events for all registered components
  // Check in reverse order (top to bottom in z-order)
  for (auto it = components.rbegin(); it != components.rend(); ++it) {
    UIComponent* component = *it;
    
    if (component->checkEvent(touch)) {
      // Component handled the event, stop propagation
      break;
    }
  }
  
  lastProcessedTouch = touch;
}

void UIManager::drawAll(bool force) {
  for (auto* component : components) {
    component->draw(force);
  }
  
  if (debugMode) {
    debugDrawBounds();
  }
}

bool UIManager::checkOverlaps() {
  bool hasOverlaps = false;
  
  for (size_t i = 0; i < components.size(); i++) {
    for (size_t j = i + 1; j < components.size(); j++) {
      if (components[i]->overlaps(*components[j])) {
        Serial.printf("[UIManager] WARNING: Overlap detected between components %d and %d\n", i, j);
        Rect bounds1 = components[i]->getBounds();
        Rect bounds2 = components[j]->getBounds();
        Serial.printf("  Component %d: (%d,%d) %dx%d\n", i, bounds1.x, bounds1.y, bounds1.w, bounds1.h);
        Serial.printf("  Component %d: (%d,%d) %dx%d\n", j, bounds2.x, bounds2.y, bounds2.w, bounds2.h);
        hasOverlaps = true;
      }
    }
  }
  
  if (hasOverlaps) {
    Serial.printf("[UIManager] Total components: %d\n", components.size());
  } else if (components.size() > 0) {
    Serial.printf("[UIManager] No overlaps detected (%d components)\n", components.size());
  }
  
  return hasOverlaps;
}

void UIManager::debugDrawBounds() {
  for (auto* component : components) {
    component->debugDraw();
  }
}
