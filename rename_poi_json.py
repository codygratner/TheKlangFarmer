import json
import glob
import os

for filepath in glob.glob('assets/controls/*.json'):
    with open(filepath, 'r') as f:
        data = json.load(f)
    
    modified = False
    for param_key, param_val in data.items():
        if "points_of_interest" in param_val:
            param_val["snap_points"] = param_val.pop("points_of_interest")
            modified = True

    if modified:
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
        print(f"Updated {filepath}")
