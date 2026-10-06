import json

filepath = 'assets/controls/filters.json'
with open(filepath, 'r') as f:
    data = json.load(f)

if "wavefolder_filter" in data:
    if "points_of_interest" in data["wavefolder_filter"]:
        del data["wavefolder_filter"]["points_of_interest"]
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
        print("Removed points_of_interest from wavefolder_filter")
