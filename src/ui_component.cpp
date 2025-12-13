#include "ui_component.h"

// UIComponent implementation
UIComponent::UIComponent(int x, int y, int w, int h) 
  : bounds(x, y, w, h) {
}

UIComponent::UIComponent(const Rect& bounds) 
  : bounds(bounds) {
}

UIComponent::~UIComponent() {
}

void UIComponent::setBounds(int x, int y, int w, int h) {
  bounds.x = x;
  bounds.y = y;
  bounds.w = w;
  bounds.h = h;
}

void UIComponent::setBounds(const Rect& newBounds) {
  bounds = newBounds;
}

void UIComponent::debugDraw() {
  if (!visible) return;
  
  // Draw bounds in debug color
  tft.drawRect(bounds.x, bounds.y, bounds.w, bounds.h, enabled ? TFT_GREEN : TFT_RED);
  tft.drawRect(bounds.x + 1, bounds.y + 1, bounds.w - 2, bounds.h - 2, enabled ? TFT_GREEN : TFT_RED);
}

// LayoutGrid implementation
LayoutGrid::LayoutGrid(int rows, int cols, int x, int y, int w, int h, int spacing)
  : rows(rows), cols(cols), x(x), y(y), w(w), h(h), spacing(spacing) {
  
  // Calculate cell dimensions accounting for spacing
  int totalSpacingW = (cols - 1) * spacing;
  int totalSpacingH = (rows - 1) * spacing;
  cellW = (w - totalSpacingW) / cols;
  cellH = (h - totalSpacingH) / rows;
}

Rect LayoutGrid::getCell(int row, int col) const {
  if (row < 0 || row >= rows || col < 0 || col >= cols) {
    return Rect(0, 0, 0, 0); // Invalid cell
  }
  
  int cellX = x + col * (cellW + spacing);
  int cellY = y + row * (cellH + spacing);
  return Rect(cellX, cellY, cellW, cellH);
}

// LayoutHelper implementation
void LayoutHelper::arrangeHorizontal(std::vector<UIComponent*> components, 
                                      int y, int totalWidth, int spacing) {
  if (components.empty()) return;
  
  int componentWidth = (totalWidth - (components.size() - 1) * spacing) / components.size();
  int x = 0;
  
  for (auto* component : components) {
    Rect bounds = component->getBounds();
    component->setBounds(x, y, componentWidth, bounds.h);
    x += componentWidth + spacing;
  }
}

void LayoutHelper::arrangeVertical(std::vector<UIComponent*> components, 
                                     int x, int startY, int spacing) {
  int y = startY;
  for (auto* component : components) {
    Rect bounds = component->getBounds();
    component->setBounds(x, y, bounds.w, bounds.h);
    y += bounds.h + spacing;
  }
}

Rect LayoutHelper::centerIn(int w, int h, const Rect& container) {
  int x = container.x + (container.w - w) / 2;
  int y = container.y + (container.h - h) / 2;
  return Rect(x, y, w, h);
}

LayoutGrid LayoutHelper::autoGrid(int rows, int cols, int x, int y, int w, int h, int spacing) {
  return LayoutGrid(rows, cols, x, y, w, h, spacing);
}
