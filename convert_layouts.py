import json

tkf = json.load(open('C:/Dev/TheKlangFarmer/assets/layouts/tkf_layout.json', 'r', encoding='utf-8'))
tkp = json.load(open('C:/Dev/TheKlangFarmer/assets/layouts/tkp_layout.json', 'r', encoding='utf-8'))

tkf_new = {
    "OSCILLATORS": {},
    "FILTERS": {},
    "ENVELOPES": {},
    "MODULATION": {},
    "MASTER FX": {}
}

# Distribute TKF cards
for k, v in tkf.items():
    if "Carrier" in k or "Modulator" in k or "Noise" in k or "Slop" in k:
        tkf_new["OSCILLATORS"][k] = v
    elif "Filter" in k and "Env" not in k:
        tkf_new["FILTERS"][k] = v
    elif "Env" in k or "Velocity" in k or "Key Track" in k:
        tkf_new["ENVELOPES"][k] = v
    elif "Mixer" in k or "Amp" in k or "Limiter" in k:
        tkf_new["MASTER FX"][k] = v
    else:
        tkf_new["MODULATION"][k] = v

tkp_new = {
    "Main": tkp
}

with open('C:/Dev/TheKlangFarmer/assets/layouts/tkf_layout.json', 'w', encoding='utf-8') as f:
    json.dump(tkf_new, f, indent=2)

with open('C:/Dev/TheKlangFarmer/assets/layouts/tkp_layout.json', 'w', encoding='utf-8') as f:
    json.dump(tkp_new, f, indent=2)
