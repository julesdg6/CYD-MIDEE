/**
 * @file lv_conf.h
 * Configuration file for LVGL v9.1.0
 * CYD MIDI Controller - LVGL Migration
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888) */
#define LV_COLOR_DEPTH 16

/*=========================
   MEMORY SETTINGS
 *=========================*/

/* Size of the memory available for `lv_malloc()` in bytes (>= 2kB) */
#define LV_MEM_SIZE (64U * 1024U)  /* 64KB */

/* Set an address for the memory pool instead of allocating it as a normal array. */
#define LV_MEM_ADR 0  /* 0: unused */

/* Use ESP32's PSRAM for LVGL memory if available */
#define LV_MEM_CUSTOM 0

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period. LVG will redraw changed areas with this period time */
#define LV_DEF_REFR_PERIOD 33  /* 30 FPS (33ms) */

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD 30  /* 33 FPS */

/*=================
   FONT USAGE
 *=================*/

/* Enable built-in fonts */
#define LV_FONT_MONTSERRAT_8  1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*===================
   THEME SETTINGS
 *===================*/

/* A simple, impressive and very complete theme */
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    /* 0: Light mode; 1: Dark mode */
    #define LV_THEME_DEFAULT_DARK 1

    /* 1: Enable grow on press */
    #define LV_THEME_DEFAULT_GROW 1

    /* Default transition time in [ms] */
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif

/*===================
   WIDGET USAGE
 *===================*/

/* Enable widgets we'll use in MIDI controller */
#define LV_USE_ANIMIMG 0
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 0
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 0
#define LV_USE_TABLE 0

/*==================
   EXAMPLES
 *==================*/

/* Enable/disable examples (for testing only, disable in production) */
#define LV_BUILD_EXAMPLES 0

/*===================
   LOGGING
 *===================*/

/* Enable the log module */
#define LV_USE_LOG 1
#if LV_USE_LOG

    /* Default log level: 
     * LV_LOG_LEVEL_TRACE, LV_LOG_LEVEL_INFO, LV_LOG_LEVEL_WARN, LV_LOG_LEVEL_ERROR, LV_LOG_LEVEL_USER, LV_LOG_LEVEL_NONE
     */
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

    /* 1: Print log with 'printf'; 0: User need to register a callback with `lv_log_register_print_cb()` */
    #define LV_LOG_PRINTF 1

    /* Enable/disable LV_LOG_TRACE in modules that produce a lot of logs */
    #define LV_LOG_TRACE_MEM            0
    #define LV_LOG_TRACE_TIMER          0
    #define LV_LOG_TRACE_INDEV          0
    #define LV_LOG_TRACE_DISP_REFR      0
    #define LV_LOG_TRACE_EVENT          0
    #define LV_LOG_TRACE_OBJ_CREATE     0
    #define LV_LOG_TRACE_LAYOUT         0
    #define LV_LOG_TRACE_ANIM           0

#endif  /* LV_USE_LOG */

/*=================
   ASSERTS
 *=================*/

/* Enable asserts if an operation is failed or an invalid data is found */
#define LV_USE_ASSERT_NULL          1   /* Check if the parameter is NULL */
#define LV_USE_ASSERT_MALLOC        1   /* Check if the memory was allocated successfully */
#define LV_USE_ASSERT_STYLE         0   /* Check if the style is properly initialized */
#define LV_USE_ASSERT_MEM_INTEGRITY 0   /* Check the integrity of `lv_mem` after critical operations */
#define LV_USE_ASSERT_OBJ           0   /* Check the object's type and existence */

/*==================
   OTHERS
 *==================*/

/* 1: Show CPU usage and FPS count in the right bottom corner */
#define LV_USE_PERF_MONITOR 1
#if LV_USE_PERF_MONITOR
    #define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT
#endif

/* 1: Show memory usage in the right top corner */
#define LV_USE_MEM_MONITOR 1
#if LV_USE_MEM_MONITOR
    #define LV_USE_MEM_MONITOR_POS LV_ALIGN_TOP_RIGHT
#endif

/* 1: Draw random colored rectangles over the redrawn areas */
#define LV_USE_REFR_DEBUG 0

/* Enable API to take snapshot for object */
#define LV_USE_SNAPSHOT 0

/* Enable Monkey test */
#define LV_USE_MONKEY 0

/* Enable grid layout */
#define LV_USE_GRID 1

/* Enable flex layout */
#define LV_USE_FLEX 1

/*==================
   DEMOS
 *==================*/

/* Disable demos for production */
#define LV_USE_DEMO_WIDGETS 0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_BENCHMARK 0
#define LV_USE_DEMO_STRESS 0
#define LV_USE_DEMO_MUSIC 0

#endif /* LV_CONF_H */
