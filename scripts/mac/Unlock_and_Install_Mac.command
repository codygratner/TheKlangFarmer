#!/bin/bash
# ==============================================================================
# The Klang Farmer & The Klang Planter - macOS One-Click Installer & Unlocker
# ==============================================================================
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo ""
echo "=================================================================="
echo "    The Klang Farmer & The Klang Planter - macOS Installer"
echo "=================================================================="
echo ""

echo "[1/4] Removing Gatekeeper quarantine flags from package..."
xattr -cr "$DIR" 2>/dev/null || true

echo "[2/4] Ensuring audio plugin directories exist..."
mkdir -p "$HOME/Library/Audio/Plug-Ins/VST3"
mkdir -p "$HOME/Library/Audio/Plug-Ins/Components"
mkdir -p "$HOME/Applications" 2>/dev/null || true

echo "[3/4] Installing plugins..."
if [ -d "$DIR/The Klang Farmer.vst3" ]; then
    echo "  -> Copying The Klang Farmer.vst3 to ~/Library/Audio/Plug-Ins/VST3/"
    cp -R -P "$DIR/The Klang Farmer.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/"
    xattr -cr "$HOME/Library/Audio/Plug-Ins/VST3/The Klang Farmer.vst3" 2>/dev/null || true
fi

if [ -d "$DIR/The Klang Planter.vst3" ]; then
    echo "  -> Copying The Klang Planter.vst3 to ~/Library/Audio/Plug-Ins/VST3/"
    cp -R -P "$DIR/The Klang Planter.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/"
    xattr -cr "$HOME/Library/Audio/Plug-Ins/VST3/The Klang Planter.vst3" 2>/dev/null || true
fi

if [ -d "$DIR/The Klang Farmer.component" ]; then
    echo "  -> Copying The Klang Farmer.component to ~/Library/Audio/Plug-Ins/Components/"
    cp -R -P "$DIR/The Klang Farmer.component" "$HOME/Library/Audio/Plug-Ins/Components/"
    xattr -cr "$HOME/Library/Audio/Plug-Ins/Components/The Klang Farmer.component" 2>/dev/null || true
fi

if [ -d "$DIR/The Klang Planter.component" ]; then
    echo "  -> Copying The Klang Planter.component to ~/Library/Audio/Plug-Ins/Components/"
    cp -R -P "$DIR/The Klang Planter.component" "$HOME/Library/Audio/Plug-Ins/Components/"
    xattr -cr "$HOME/Library/Audio/Plug-Ins/Components/The Klang Planter.component" 2>/dev/null || true
fi

if [ -d "$DIR/The Klang Farmer.app" ]; then
    echo "  -> Copying The Klang Farmer.app to /Applications/"
    cp -R -P "$DIR/The Klang Farmer.app" "/Applications/" 2>/dev/null || cp -R -P "$DIR/The Klang Farmer.app" "$HOME/Applications/" 2>/dev/null || true
    xattr -cr "/Applications/The Klang Farmer.app" 2>/dev/null || true
fi

if [ -d "$DIR/The Klang Planter.app" ]; then
    echo "  -> Copying The Klang Planter.app to /Applications/"
    cp -R -P "$DIR/The Klang Planter.app" "/Applications/" 2>/dev/null || cp -R -P "$DIR/The Klang Planter.app" "$HOME/Applications/" 2>/dev/null || true
    xattr -cr "/Applications/The Klang Planter.app" 2>/dev/null || true
fi

echo "[4/4] Refreshing CoreAudio daemon (AU cache update)..."
killall -9 coreaudiod 2>/dev/null || true

echo ""
echo "=================================================================="
echo "  SUCCESS: The Klang Farmer & The Klang Planter are installed!"
echo "  Formats: AU, VST3, Standalone"
echo "  Launch your DAW (Logic Pro, Ableton Live, Reaper, FL Studio)"
echo "  and rescan your plugins."
echo "=================================================================="
echo ""
