import json
import glob
import os

for filepath in glob.glob('assets/controls/*.json'):
    with open(filepath, 'r') as f:
        data = json.load(f)
    
    modified = False
    for param_key, param_val in data.items():
        if "snap_points" in param_val and "shape" in param_key:
            if "format" not in param_val or param_val["format"] != "waveshape":
                param_val["format"] = "waveshape"
                modified = True

    if modified:
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
        print(f"Updated {filepath}")
