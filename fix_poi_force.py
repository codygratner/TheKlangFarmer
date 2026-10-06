import json
import glob

new_poi = [
    {"value": 0.0, "label": "Sine"},
    {"value": 0.2, "label": "Triangle"},
    {"value": 0.4, "label": "Sawtooth"},
    {"value": 0.6, "label": "Square"},
    {"value": 1.0, "label": "PWM"}
]

shape_params = [
    "carrier1_shape",
    "carrier2_shape",
    "mod1_shape",
    "mod2_shape",
    "ringmod_shape",
    "planter_carrier_shape",
    "planter_mod_shape"
]

for filepath in glob.glob('assets/controls/*.json'):
    with open(filepath, 'r') as f:
        data = json.load(f)
    
    modified = False
    for param_key, param_val in data.items():
        if param_key in shape_params:
            param_val["points_of_interest"] = new_poi
            modified = True
            print(f"Updated {param_key} in {filepath}")
            
    if modified:
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
