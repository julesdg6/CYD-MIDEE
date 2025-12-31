#ifndef CYD_HARDWARE_H
#define CYD_HARDWARE_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

/**
 * CYD Hardware Abstraction Layer
 * 
 * Provides unified configuration for all CYD board variants (2.4", 2.8", 3.5")
 * Based on ESP32-Cheap-Yellow-Display project best practices:
 * https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display
 * 
 * This replaces manual rotation detection and pin management with a
 * centralized configuration that automatically adapts to each board variant.
 */

// ============================================================================
// Board Variant Detection
// ============================================================================

// Detect which board variant we're building for based on build flags
#if defined(ILI9488_DRIVER)
  #define CYD_BOARD_35  // 3.5" display (480x320)
#elif defined(ILI9341_DRIVER) || defined(ILI9341_2_DRIVER)
  // Distinguish between 2.8" and 2.4" by backlight pin
  #if (TFT_BL == 21)
    #define CYD_BOARD_28  // 2.8" display (320x240)
  #else
    #define CYD_BOARD_24  // 2.4" display (320x240)
  #endif
#else
  #define CYD_BOARD_35  // Default to 3.5" if not specified
#endif

// ============================================================================
// Display Configuration
// ============================================================================

// Board-specific display settings
#ifdef CYD_BOARD_35
  // CYD 3.5" - ILI9488 480x320 landscape
  #define CYD_DISPLAY_WIDTH      480
  #define CYD_DISPLAY_HEIGHT     320
  #define CYD_DISPLAY_ROTATION   1    // Landscape orientation
  #define CYD_TOUCH_ROTATION     1    // Match display
  #define CYD_BACKLIGHT_PIN      27
  #define CYD_BOARD_NAME         "CYD 3.5\""
  
#elif defined(CYD_BOARD_28)
  // CYD 2.8" - ILI9341 320x240 landscape
  #define CYD_DISPLAY_WIDTH      320
  #define CYD_DISPLAY_HEIGHT     240
  #define CYD_DISPLAY_ROTATION   0    // Portrait upright (becomes landscape with TFT_WIDTH swap)
  #define CYD_TOUCH_ROTATION     0    // Match display
  #define CYD_BACKLIGHT_PIN      21
  #define CYD_BOARD_NAME         "CYD 2.8\""
  
#elif defined(CYD_BOARD_24)
  // CYD 2.4" - ILI9341 320x240 landscape
  #define CYD_DISPLAY_WIDTH      320
  #define CYD_DISPLAY_HEIGHT     240
  #define CYD_DISPLAY_ROTATION   0    // Portrait upright (becomes landscape with TFT_WIDTH swap)
  #define CYD_TOUCH_ROTATION     0    // Match display
  #define CYD_BACKLIGHT_PIN      21
  #define CYD_BOARD_NAME         "CYD 2.4\""
  
#endif

// ============================================================================
// Touchscreen Pin Definitions (Common to All CYD Boards)
// ============================================================================

#define CYD_TOUCH_IRQ          36
#define CYD_TOUCH_MOSI         32
#define CYD_TOUCH_MISO         39
#define CYD_TOUCH_CLK          25
#define CYD_TOUCH_CS           33

// ============================================================================
// SD Card Pin Definitions (Common to All CYD Boards)
// ============================================================================

#define CYD_SD_CS              5
#define CYD_SD_MOSI            23
#define CYD_SD_MISO            19
#define CYD_SD_SCK             18

// ============================================================================
// Other Peripherals
// ============================================================================

#define CYD_RGB_LED_RED        4     // RGB LED pins (if available)
#define CYD_RGB_LED_GREEN      16
#define CYD_RGB_LED_BLUE       17
#define CYD_LDR_PIN            34    // Light dependent resistor
#define CYD_SPEAKER_PIN        26    // Speaker/buzzer (if available)

// ============================================================================
// Free GPIO Pins (Available for External Hardware)
// ============================================================================

// These pins are available for user projects:
// GPIO 22, GPIO 27 (if not using 3.5" backlight), GPIO 35
// See BUILD.md for complete pin reference

// ============================================================================
// Hardware Initialization Helper Class
// ============================================================================

/**
 * CYD Hardware Manager
 * Provides unified initialization and configuration for all CYD variants
 */
class CYDHardware {
public:
  /**
   * Get the display rotation value for current board variant
   */
  static uint8_t getDisplayRotation() {
    return CYD_DISPLAY_ROTATION;
  }
  
  /**
   * Get the touch rotation value for current board variant
   */
  static uint8_t getTouchRotation() {
    return CYD_TOUCH_ROTATION;
  }
  
  /**
   * Get the display width for current board variant
   */
  static uint16_t getDisplayWidth() {
    return CYD_DISPLAY_WIDTH;
  }
  
  /**
   * Get the display height for current board variant
   */
  static uint16_t getDisplayHeight() {
    return CYD_DISPLAY_HEIGHT;
  }
  
  /**
   * Get the backlight pin for current board variant
   */
  static uint8_t getBacklightPin() {
    return CYD_BACKLIGHT_PIN;
  }
  
  /**
   * Get human-readable board name
   */
  static const char* getBoardName() {
    return CYD_BOARD_NAME;
  }
  
  /**
   * Initialize display with correct rotation
   */
  static void initDisplay(TFT_eSPI& tft) {
    tft.init();
    tft.setRotation(CYD_DISPLAY_ROTATION);
    Serial.printf("Display initialized: %s (%dx%d, rotation %d)\n", 
                  CYD_BOARD_NAME, CYD_DISPLAY_WIDTH, CYD_DISPLAY_HEIGHT, CYD_DISPLAY_ROTATION);
  }
  
  /**
   * Initialize touchscreen with correct rotation
   */
  static void initTouch(XPT2046_Touchscreen& ts, SPIClass& spi) {
    spi.begin(CYD_TOUCH_CLK, CYD_TOUCH_MISO, CYD_TOUCH_MOSI, CYD_TOUCH_CS);
    ts.begin(spi);
    ts.setRotation(CYD_TOUCH_ROTATION);
    Serial.printf("Touch initialized: rotation %d\n", CYD_TOUCH_ROTATION);
  }
  
  /**
   * Initialize SD card with correct pins
   */
  static bool initSD(SPIClass& spi) {
    spi.begin(CYD_SD_SCK, CYD_SD_MISO, CYD_SD_MOSI, CYD_SD_CS);
    bool success = SD.begin(CYD_SD_CS, spi, 1000000);
    if (success) {
      Serial.println("SD card initialized successfully");
    } else {
      Serial.println("SD card initialization failed");
    }
    return success;
  }
  
  /**
   * Set backlight brightness (0-255)
   */
  static void setBacklight(uint8_t brightness) {
    analogWrite(CYD_BACKLIGHT_PIN, brightness);
  }
  
  /**
   * Turn backlight on/off
   */
  static void setBacklight(bool on) {
    digitalWrite(CYD_BACKLIGHT_PIN, on ? HIGH : LOW);
  }
  
  /**
   * Print hardware configuration info to Serial
   */
  static void printInfo() {
    Serial.println("\n=== CYD Hardware Configuration ===");
    Serial.printf("Board: %s\n", CYD_BOARD_NAME);
    Serial.printf("Display: %dx%d (rotation %d)\n", 
                  CYD_DISPLAY_WIDTH, CYD_DISPLAY_HEIGHT, CYD_DISPLAY_ROTATION);
    Serial.printf("Touch: rotation %d\n", CYD_TOUCH_ROTATION);
    Serial.printf("Backlight pin: %d\n", CYD_BACKLIGHT_PIN);
    Serial.printf("Touch pins - CS:%d MOSI:%d MISO:%d CLK:%d IRQ:%d\n",
                  CYD_TOUCH_CS, CYD_TOUCH_MOSI, CYD_TOUCH_MISO, 
                  CYD_TOUCH_CLK, CYD_TOUCH_IRQ);
    Serial.printf("SD pins - CS:%d MOSI:%d MISO:%d SCK:%d\n",
                  CYD_SD_CS, CYD_SD_MOSI, CYD_SD_MISO, CYD_SD_SCK);
    Serial.println("==================================\n");
  }
};

// ============================================================================
// Legacy Compatibility Macros
// ============================================================================

// For backward compatibility with existing code that uses these names
#define XPT2046_IRQ    CYD_TOUCH_IRQ
#define XPT2046_MOSI   CYD_TOUCH_MOSI
#define XPT2046_MISO   CYD_TOUCH_MISO
#define XPT2046_CLK    CYD_TOUCH_CLK
#define XPT2046_CS     CYD_TOUCH_CS

#define SD_CS          CYD_SD_CS
#define SD_MOSI        CYD_SD_MOSI
#define SD_MISO        CYD_SD_MISO
#define SD_SCK         CYD_SD_SCK

#endif // CYD_HARDWARE_H
