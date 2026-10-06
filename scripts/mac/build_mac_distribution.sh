#!/bin/bash
# ==============================================================================
# The Klang Farmer & The Klang Planter - macOS Distribution Packager
# Generates .pkg Installer, stylized .dmg, and portable .zip archive
# ==============================================================================
set -e

TAG="${1:-v0.3.0}"
BUILD_DIR="${2:-build}"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

echo "========================================================================"
echo " Packaging macOS Distribution for Release: $TAG"
echo " Build directory: $BUILD_DIR"
echo "========================================================================"

# 1. Clean and prepare directories
rm -rf pkg_root pkg_scripts dmg_root macos_dist
mkdir -p pkg_root/Library/Audio/Plug-Ins/VST3
mkdir -p pkg_root/Library/Audio/Plug-Ins/Components
mkdir -p pkg_root/Applications
mkdir -p pkg_scripts
mkdir -p dmg_root
mkdir -p macos_dist

# 2. Locate built artifacts
find "$BUILD_DIR" -name "The Klang Farmer.vst3" -exec cp -R -P {} pkg_root/Library/Audio/Plug-Ins/VST3/ \;
find "$BUILD_DIR" -name "The Klang Farmer.component" -exec cp -R -P {} pkg_root/Library/Audio/Plug-Ins/Components/ \;
find "$BUILD_DIR" -name "The Klang Planter.vst3" -exec cp -R -P {} pkg_root/Library/Audio/Plug-Ins/VST3/ \;
find "$BUILD_DIR" -name "The Klang Planter.component" -exec cp -R -P {} pkg_root/Library/Audio/Plug-Ins/Components/ \;
find "$BUILD_DIR" -name "The Klang Farmer.app" -exec cp -R -P {} pkg_root/Applications/ \;
find "$BUILD_DIR" -name "The Klang Planter.app" -exec cp -R -P {} pkg_root/Applications/ \;
find "$BUILD_DIR" -name "The Klang Editor.app" -exec cp -R -P {} pkg_root/Applications/ \;

# 3. Ad-hoc Codesign all binaries
echo "Codesigning binaries..."
find pkg_root -name "*.vst3" -o -name "*.component" -o -name "*.app" | while read -r item; do
    echo "  Ad-hoc signing: $item"
    codesign --force --deep -s - "$item" 2>/dev/null || true
done

# 4. Generate .pkg Installer with automated postinstall quarantine stripper
echo "Building .pkg Installer..."
cp "$SCRIPT_DIR/postinstall" pkg_scripts/
chmod +x pkg_scripts/postinstall

CLEAN_VER="${TAG#v}"
pkgbuild --root pkg_root \
         --identifier com.rlyehsound.theklangfarmer.pkg \
         --version "$CLEAN_VER" \
         --install-location "/" \
         --scripts pkg_scripts \
         "TheKlangFarmer-component.pkg"

productbuild --package "TheKlangFarmer-component.pkg" \
             "TheKlangFarmer-${TAG}-macOS-Installer.pkg"

rm -f "TheKlangFarmer-component.pkg"
echo "  SUCCESS: TheKlangFarmer-${TAG}-macOS-Installer.pkg generated."

# 5. Generate stylized .dmg with symlinks & visual guide
echo "Building portable .dmg..."
cp -R -P pkg_root/Library/Audio/Plug-Ins/VST3/* dmg_root/
cp -R -P pkg_root/Library/Audio/Plug-Ins/Components/* dmg_root/
cp -R -P pkg_root/Applications/* dmg_root/

ln -s "/Library/Audio/Plug-Ins/VST3" dmg_root/"Drag to VST3 Folder"
ln -s "/Library/Audio/Plug-Ins/Components" dmg_root/"Drag to AU Components Folder"
ln -s "/Applications" dmg_root/"Drag to Applications"

cp "$SCRIPT_DIR/Fix_Mac_Permissions.command" dmg_root/
cp "$SCRIPT_DIR/macOS_Install_Guide.html" dmg_root/
chmod +x dmg_root/Fix_Mac_Permissions.command

hdiutil create -volname "The Klang Farmer" \
               -srcfolder dmg_root \
               -ov \
               -format UDZO \
               "TheKlangFarmer-${TAG}-macOS.dmg"

echo "  SUCCESS: TheKlangFarmer-${TAG}-macOS.dmg generated."

# 6. Generate portable .zip archive
echo "Building portable .zip..."
cp -R dmg_root/* macos_dist/
cp "$SCRIPT_DIR/README_MAC_INSTALL.txt" macos_dist/ 2>/dev/null || true
cd macos_dist
zip -r -y "../TheKlangFarmer-${TAG}-macOS.zip" *
cd ..
echo "  SUCCESS: TheKlangFarmer-${TAG}-macOS.zip generated."

# Clean temporary folders
rm -rf pkg_root pkg_scripts dmg_root macos_dist

echo "========================================================================"
echo " macOS Release Packages Ready:"
ls -lh TheKlangFarmer-${TAG}-macOS*
echo "========================================================================"
