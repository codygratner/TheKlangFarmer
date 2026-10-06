import re, json
tkp = {
  "Carrier": {
    "color": "colRed",
    "style": "StandardDark",
    "parameters": ["carrier_tracking", "carrier_pitch", "carrier_shape", "carrier_depth"]
  },
  "Modulator": {
    "color": "colCyan",
    "style": "StandardDark",
    "parameters": ["mod_type", "mod_shape", "mod_ratio", "mod_depth"]
  },
  "Pitch Env": {
    "color": "colSilver",
    "style": "StandardDark",
    "parameters": ["pitch_env_slope", "pitch_env_depth", "pitch_env_decay"]
  },
  "Noise Transient": {
    "color": "colDarkGrey",
    "style": "StandardDark",
    "parameters": ["noise_rate", "noise_color", "noise_decay", "noise_crossfade"]
  },
  "Filter": {
    "color": "colBlue",
    "style": "StandardDark",
    "parameters": ["filter_type", "filter_cutoff", "filter_res"]
  },
  "Filter Env": {
    "color": "colAmber",
    "style": "StandardDark",
    "parameters": ["filter_env_slope", "filter_env_depth", "filter_env_decay", "filter_drive"]
  },
  "Amplifier": {
    "color": "colGreen",
    "style": "StandardDark",
    "parameters": ["amp_drive", "amp_pan", "amp_vel_slope", "amp_vel_floor"]
  },
  "Amp Envelope": {
    "color": "colMagenta",
    "style": "StandardDark",
    "parameters": ["amp_env_claps", "amp_env_clap_speed", "amp_env_slope", "amp_env_decay"]
  }
}
with open('C:/Dev/TheKlangFarmer/assets/layouts/tkp_layout.json', 'w', encoding='utf-8') as f:
    json.dump(tkp, f, indent=2)

tkf = json.load(open('C:/Dev/TheKlangFarmer/assets/layouts/tkf_layout.json', 'r', encoding='utf-8'))
tkf["Mod Env 1"] = {
    "color": "colAmber", "style": "StandardDark", "parameters": ["mod_env1_target", "mod_env1_slope", "mod_env1_depth", "mod_env1_decay"]
}
tkf["Mod Env 2"] = {
    "color": "colAmber", "style": "StandardDark", "parameters": ["mod_env2_target", "mod_env2_slope", "mod_env2_depth", "mod_env2_decay"]
}
tkf["Mod Env 3"] = {
    "color": "colAmber", "style": "StandardDark", "parameters": ["mod_env3_target", "mod_env3_slope", "mod_env3_depth", "mod_env3_decay"]
}
with open('C:/Dev/TheKlangFarmer/assets/layouts/tkf_layout.json', 'w', encoding='utf-8') as f:
    json.dump(tkf, f, indent=2)
