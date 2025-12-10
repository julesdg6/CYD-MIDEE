#!/bin/bash

# Download all screenshots from CYD-MIDI-Controller SD card
# Usage: ./download_screenshots.sh <IP_ADDRESS>
# Example: ./download_screenshots.sh 192.168.1.244

if [ -z "$1" ]; then
    echo "Usage: $0 <IP_ADDRESS>"
    echo "Example: $0 192.168.1.244"
    exit 1
fi

IP=$1
ASSETS_DIR="assets"

# Create assets directory if it doesn't exist
mkdir -p "$ASSETS_DIR"

echo "Downloading screenshots from http://$IP/screenshots/"
echo "Saving to $ASSETS_DIR/"
echo ""

# List of all screenshot files (3 menus + 15 modes = 18 total)
FILES=(
    "00_main_menu.bmp"
    "01_settings_menu.bmp"
    "02_bluetooth_status.bmp"
    "03_keyboard.bmp"
    "04_sequencer.bmp"
    "05_bouncing_ball.bmp"
    "06_physics_drop.bmp"
    "07_random_gen.bmp"
    "08_xy_pad.bmp"
    "09_arpeggiator.bmp"
    "10_grid_piano.bmp"
    "11_auto_chord.bmp"
    "12_lfo.bmp"
    "13_tb3po.bmp"
    "14_grids.bmp"
    "15_raga.bmp"
    "16_euclidean.bmp"
    "17_morph.bmp"
)

SUCCESS_COUNT=0
FAIL_COUNT=0

# Download each file
for FILE in "${FILES[@]}"; do
    URL="http://$IP/download?file=/screenshots/$FILE"
    OUTPUT="$ASSETS_DIR/$FILE"
    
    echo -n "Downloading $FILE... "
    
    if wget -q -O "$OUTPUT" "$URL" 2>/dev/null; then
        echo "✓"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo "✗ Failed"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
done

echo ""
echo "================================"
echo "Downloaded: $SUCCESS_COUNT files"
if [ $FAIL_COUNT -gt 0 ]; then
    echo "Failed: $FAIL_COUNT files"
fi
echo "Location: $ASSETS_DIR/"
echo "================================"
