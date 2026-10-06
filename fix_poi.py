import json
import glob
import os

expected_old = [
    {"value": 0.0, "label": "Sine"},
    {"value": 0.25, "label": "Triangle"},
    {"value": 0.5, "label": "Sawtooth"},
    {"value": 0.75, "label": "Square"},
    {"value": 1.0, "label": "PWM"}
]

new_poi = [
    {"value": 0.0, "label": "Sine"},
    {"value": 0.2, "label": "Triangle"},
    {"value": 0.4, "label": "Sawtooth"},
    {"value": 0.6, "label": "Square"},
    {"value": 1.0, "label": "PWM"}
]

for filepath in glob.glob('assets/controls/*.json'):
    with open(filepath, 'r') as f:
        data = json.load(f)
    
    modified = False
    for param_key, param_val in data.items():
        if "points_of_interest" in param_val:
            poi = param_val["points_of_interest"]
            # Check if this matches the shape POI
            labels = [p.get("label") for p in poi]
            if labels == ["Sine", "Triangle", "Sawtooth", "Square", "PWM"]:
                param_val["points_of_interest"] = new_poi
                modified = True
                print(f"Updated {param_key} in {filepath}")

    if modified:
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
