/*******************************************************************
 LVGL Test Mode - Phase 1.3 Hardware Validation
 Tests LVGL + esp32-smartdisplay initialization
 Validates display rendering and touch input
 *******************************************************************/

#ifndef LVGL_TEST_MODE_H
#define LVGL_TEST_MODE_H

#include "common_definitions.h"
#include <lvgl.h>

// LVGL objects for test UI
static lv_obj_t* test_screen = nullptr;
static lv_obj_t* test_label = nullptr;
static lv_obj_t* test_button = nullptr;
static lv_obj_t* test_button_label = nullptr;
static lv_obj_t* test_touch_label = nullptr;
static int button_press_count = 0;

// Button event handler
static void button_event_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  
  if (code == LV_EVENT_CLICKED) {
    button_press_count++;
    
    // Update button label
    String text = "Pressed " + String(button_press_count) + " times";
    lv_label_set_text(test_button_label, text.c_str());
    
    Serial.printf("LVGL Test Button clicked! Count: %d\n", button_press_count);
  }
}

// Initialize LVGL test mode
void initializeLVGLTestMode() {
  Serial.println("\n=== LVGL Test Mode Initialize ===");
  
  // Reset counter
  button_press_count = 0;
  
  // Create test screen
  test_screen = lv_obj_create(NULL);
  lv_scr_load(test_screen);
  
  // Set background color to match theme
  lv_obj_set_style_bg_color(test_screen, lv_color_hex(THEME_BG), 0);
  
  // Create header label
  test_label = lv_label_create(test_screen);
  lv_label_set_text(test_label, "LVGL HARDWARE TEST");
  lv_obj_set_style_text_color(test_label, lv_color_hex(THEME_PRIMARY), 0);
  lv_obj_align(test_label, LV_ALIGN_TOP_MID, 0, 10);
  
  // Create test button
  test_button = lv_btn_create(test_screen);
  lv_obj_set_size(test_button, 200, 60);
  lv_obj_align(test_button, LV_ALIGN_CENTER, 0, -20);
  lv_obj_set_style_bg_color(test_button, lv_color_hex(THEME_PRIMARY), 0);
  lv_obj_add_event_cb(test_button, button_event_handler, LV_EVENT_CLICKED, NULL);
  
  // Button label
  test_button_label = lv_label_create(test_button);
  lv_label_set_text(test_button_label, "Click Me!");
  lv_obj_center(test_button_label);
  
  // Touch coordinate label
  test_touch_label = lv_label_create(test_screen);
  lv_label_set_text(test_touch_label, "Touch: (0, 0)");
  lv_obj_set_style_text_color(test_touch_label, lv_color_hex(THEME_TEXT_DIM), 0);
  lv_obj_align(test_touch_label, LV_ALIGN_BOTTOM_MID, 0, -60);
  
  // Info label
  lv_obj_t* info_label = lv_label_create(test_screen);
  lv_label_set_text(info_label, 
    "Testing LVGL display + touch\n"
    "esp32-smartdisplay library\n"
    "Press button to test touch input");
  lv_obj_set_style_text_color(info_label, lv_color_hex(THEME_TEXT), 0);
  lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(info_label, LV_ALIGN_CENTER, 0, 60);
  
  // Back button (top-left, matching other modes)
  lv_obj_t* back_btn = lv_btn_create(test_screen);
  lv_obj_set_size(back_btn, 70, 40);
  lv_obj_set_pos(back_btn, 5, 5);
  lv_obj_set_style_bg_color(back_btn, lv_color_hex(THEME_SECONDARY), 0);
  
  lv_obj_t* back_label = lv_label_create(back_btn);
  lv_label_set_text(back_label, "BACK");
  lv_obj_center(back_label);
  
  // Back button event - exit to menu
  lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
      Serial.println("LVGL Test Mode: Back button pressed, exiting to menu");
      // Note: exitToMenu() will be called from main loop
      currentMode = MENU;
    }
  }, LV_EVENT_CLICKED, NULL);
  
  Serial.println("LVGL Test Mode initialized successfully");
}

// Draw LVGL test mode (called once on mode entry)
void drawLVGLTestMode() {
  // LVGL handles all drawing - nothing to do here
  Serial.println("LVGL Test Mode: Screen created and loaded");
}

// Handle LVGL test mode (called in main loop)
void handleLVGLTestMode() {
  // Update touch coordinate label (for debugging)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 100) {  // Update every 100ms
    // Get touch state from LVGL input device
    lv_indev_t* indev = lv_indev_get_next(NULL);
    if (indev != nullptr) {
      lv_point_t point;
      lv_indev_get_point(indev, &point);
      
      if (lv_indev_get_state(indev) == LV_INDEV_STATE_PRESSED) {
        String coords = "Touch: (" + String(point.x) + ", " + String(point.y) + ")";
        lv_label_set_text(test_touch_label, coords.c_str());
      }
    }
    
    lastUpdate = millis();
  }
  
  // LVGL task handler is called in main loop, so nothing else needed here
}

// Cleanup when exiting mode
void cleanupLVGLTestMode() {
  if (test_screen != nullptr) {
    lv_obj_del(test_screen);
    test_screen = nullptr;
    test_label = nullptr;
    test_button = nullptr;
    test_button_label = nullptr;
    test_touch_label = nullptr;
  }
  
  Serial.println("LVGL Test Mode cleaned up");
}

#endif // LVGL_TEST_MODE_H
