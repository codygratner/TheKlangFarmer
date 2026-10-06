import os
import glob

replacements = {
    "points_of_interest": "snap_points",
    "PointOfInterest": "SnapPoint",
    "pointsOfInterest": "snapPoints"
}

files_to_check = glob.glob('source/**/*.cpp', recursive=True) + \
                 glob.glob('source/**/*.h', recursive=True) + \
                 glob.glob('tools/**/*.cpp', recursive=True) + \
                 glob.glob('tools/**/*.h', recursive=True)

for filepath in files_to_check:
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    for old, new in replacements.items():
        content = content.replace(old, new)
        
    if content != original:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Updated {filepath}")
