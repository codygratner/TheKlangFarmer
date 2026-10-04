================================================================================
       The Klang Farmer & The Klang Planter - macOS Installation Guide
================================================================================

Compatibility:
- macOS 11.0 (Big Sur) or later
- Universal Binary: Native Apple Silicon (M1/M2/M3/M4) & Intel x86_64
- Formats: AU (AudioUnit .component), VST3, Standalone App

--------------------------------------------------------------------------------
OPTION 1: One-Click Automatic Install (Recommended)
--------------------------------------------------------------------------------
1. Double-click "Unlock_and_Install_Mac.command".
2. If macOS prompts that the script cannot be opened:
   - Right-click (or Control-click) "Unlock_and_Install_Mac.command"
   - Select "Open", then click "Open" in the prompt dialog.
3. The script will automatically:
   - Remove Gatekeeper quarantine attributes
   - Install VST3 plugins to: ~/Library/Audio/Plug-Ins/VST3/
   - Install AU plugins to:   ~/Library/Audio/Plug-Ins/Components/
   - Install Standalone apps to: /Applications/
   - Refresh the macOS CoreAudio daemon so Logic Pro and DAWs detect them immediately.

--------------------------------------------------------------------------------
OPTION 2: Manual Installation
--------------------------------------------------------------------------------
1. Copy VST3 plugins to your user VST3 directory:
   "The Klang Farmer.vst3"  -> ~/Library/Audio/Plug-Ins/VST3/
   "The Klang Planter.vst3" -> ~/Library/Audio/Plug-Ins/VST3/

2. Copy AU components to your user Components directory:
   "The Klang Farmer.component"  -> ~/Library/Audio/Plug-Ins/Components/
   "The Klang Planter.component" -> ~/Library/Audio/Plug-Ins/Components/

3. Copy Standalone applications to /Applications/
   "The Klang Farmer.app"  -> /Applications/
   "The Klang Planter.app" -> /Applications/

4. Open Terminal and run this command to unquarantine the installed files:
   xattr -cr ~/Library/Audio/Plug-Ins/VST3/The\ Klang*
   xattr -cr ~/Library/Audio/Plug-Ins/Components/The\ Klang*
   killall -9 coreaudiod

--------------------------------------------------------------------------------
DAW Verification Notes:
--------------------------------------------------------------------------------
- Logic Pro: Logic scans plugins automatically on startup. If needed, go to:
  Settings -> Plug-in Manager -> Reset & Rescan Selection.
- Ableton Live: Preferences -> Plug-Ins -> Rescan.
- Reaper: Preferences -> VST -> Rescan.
- FL Studio: Manage Plugins -> Find Installed Plugins.
