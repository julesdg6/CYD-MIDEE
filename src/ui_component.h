#ifndef UI_COMPONENT_H
#define UI_COMPONENT_H

#include "common_definitions.h"
#include <vector>
#include <functional>

// Forward declarations
class UIManager;

// Rectangle helper struct
struct Rect {
  int x, y, w, h;
  
  Rect() : x(0), y(0), w(0), h(0) {}
  Rect(int _x, int _y, int _w, int _h) : x(_x), y(_y), w(_w), h(_h) {}
  
  int right() const { return x + w; }
  int bottom() const { return y + h; }
  int centerX() const { return x + w / 2; }
  int centerY() const { return y + h / 2; }
  
  bool contains(int px, int py) const {
    return px >= x && px <= x + w && py >= y && py <= y + h;
  }
  
  bool overlaps(const Rect& other) const {
    return !(x > other.x + other.w || 
             x + w < other.x || 
             y > other.y + other.h || 
             y + h < other.y);
  }
};

// Base class for all UI components
class UIComponent {
public:
  UIComponent(int x, int y, int w, int h);
  UIComponent(const Rect& bounds);
  virtual ~UIComponent();
  
  // Core API - must be implemented by derived classes
  virtual void draw(bool force = false) = 0;
  virtual bool checkEvent(const TouchState& touch) = 0;
  
  // Bounds management
  virtual void setBounds(int x, int y, int w, int h);
  virtual void setBounds(const Rect& bounds);
  Rect getBounds() const { return bounds; }
  
  // Hit testing
  bool contains(int x, int y) const { return bounds.contains(x, y); }
  bool overlaps(const UIComponent& other) const { return bounds.overlaps(other.getBounds()); }
  
  // State management
  void setEnabled(bool enabled) { this->enabled = enabled; }
  bool isEnabled() const { return enabled; }
  void setVisible(bool visible) { this->visible = visible; }
  bool isVisible() const { return visible; }
  
  // Debug
  void debugDraw();
  
protected:
  Rect bounds;
  bool enabled = true;
  bool visible = true;
  bool lastDrawnPressed = false;
};

// Global references (defined in main .ino file)
extern TFT_eSPI tft;
extern TouchState touch;

// Layout grid helper for consistent spacing
class LayoutGrid {
public:
  LayoutGrid(int rows, int cols, int x, int y, int w, int h, int spacing);
  
  Rect getCell(int row, int col) const;
  int cellWidth() const { return cellW; }
  int cellHeight() const { return cellH; }
  int bottom() const { return y + h; }
  int right() const { return x + w; }
  
private:
  int rows, cols, x, y, w, h, spacing;
  int cellW, cellH;
};

// Layout helper utilities
class LayoutHelper {
public:
  // Arrange components horizontally with equal spacing
  static void arrangeHorizontal(std::vector<UIComponent*> components, 
                                 int y, int totalWidth, int spacing);
  
  // Arrange components vertically with equal spacing
  static void arrangeVertical(std::vector<UIComponent*> components, 
                               int x, int startY, int spacing);
  
  // Center a component in a container
  static Rect centerIn(int w, int h, const Rect& container);
  
  // Calculate responsive grid (auto-size cells to fill space)
  static LayoutGrid autoGrid(int rows, int cols, int x, int y, int w, int h, int spacing);
};

#endif // UI_COMPONENT_H
