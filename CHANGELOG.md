# Changelog

## [Unreleased] - LVGL Migration in Progress

### Phase 1.2: Build System Migration - ✅ COMPLETE (2025-12-31)
- **Added** LVGL v9.1.0 dependency to platformio.ini
- **Added** esp32-smartdisplay v3.0.8 library for hardware abstraction
- **Created** `src/lv_conf.h` configuration file (RGB565, 64KB heap, 30 FPS)
- **Configured** LVGL build flags and memory settings
- **Verified** compilation successful on cyd28 (1.73MB binary, 76.6KB RAM)
- **Maintained** backward compatibility with legacy TFT_eSPI during migration
- **Documentation**: See `docs/PHASE_1_2_COMPLETION.md` for full details

### Migration Status
- Phase 1.1: LVGL Learning & Environment Setup - ⏳ In Progress
- Phase 1.2: Build System Migration - ✅ Complete
- Phase 1.3: Hardware Initialization Refactor - ⏳ Next
- For full migration plan, see `LVGL_MIGRATION_PLAN.md`

---

## [2.0.0] - 2025-12-10

### Major Features Added
- **5 New Advanced Modes** - TB3PO (acid bassline), GRIDS (Euclidean drums), RAGA (Indian classical), EUCLIDEAN (pure rhythms), MORPH (gesture synthesis)
- **WiFi Web Server** - Full-featured web interface for file management and remote control
- **Expanded Flash Memory** - Increased from 1.3MB to 3.1MB using huge_app partition (54.4% usage)
- **Screenshot System** - Capture all 15 modes + 3 menu screens (18 total) to SD card
- **Persistent WiFi Config** - Store credentials on SD card for automatic connection

### Web Server Features
- Directory navigation with breadcrumb UI
- File upload/download/delete operations
- Real-time screenshot capture via `/screenshot` endpoint
- WiFi configuration via web form
- Automatic STA/AP mode fallback

### Enhancements
- TB3PO icon changed to transparent outline (acid smiley) on yellow background
- Screenshot routine cycles through all 15 modes and 3 menus
- SD card remounting fixed for reliable multi-screenshot capture
- URLs displayed on-screen and Serial Monitor during screenshot capture
- Download script for batch screenshot retrieval

### Technical Improvements
- Partition scheme: huge_app.csv (3MB app space)
- RAM usage: 23.2% (75,976 bytes)
- Flash usage: 54.4% (1,710,105 bytes of 3,145,728 bytes)
- Web server port: 80 (HTTP)
- AP mode: "CYD-MIDI" / "midi1234"

### Bug Fixes
- Fixed SD card VFS registration errors during screenshot capture
- Resolved coordinate mismatches in touch detection
- Improved touch target sizes (minimum 45px)

## [1.0.0] - Original Release

### Initial Features
- 10 interactive MIDI modes
- Bluetooth MIDI support
- Touchscreen calibration
- Basic SD card support
- Visual feedback system
