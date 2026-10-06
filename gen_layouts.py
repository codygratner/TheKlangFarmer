import re

def parse_cpp(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        content = f.read()
    
    cards = {}
    
    # Simple regex to find blocks of code for each card
    # In FarmerEditor.cpp, they are numbered like: // 1. Carrier 1
    blocks = re.split(r'// \d+\. ', content)
    for block in blocks[1:]:
        lines = block.strip().split('\n')
        title_line = lines[0]
        
        # Find card instantiation
        card_match = re.search(r'std::make_unique<ModuleCardComponent>\("([^"]+)", juce::Colour\((0x[0-9a-fA-F]+)\)(?:,\s*ModuleCardComponent::PanelStyle::([A-Za-z]+))?\)', block)
        if not card_match:
            # Special case for Mod Envelopes loop or lambda
            continue
            
        title = card_match.group(1)
        color_hex = card_match.group(2)
        style = card_match.group(3) if card_match.group(3) else "StandardDark"
        
        # Infer color ID from hex (rough mapping)
        # 0xff00e5ff colCyan
        # 0xffe53935 colRed
        # 0xff43a047 colGreen
        # 0xffab47bc colPurple
        # 0xffffb300 colAmber
        # 0xff1e88e5 colBlue
        # 0xfffb8c00 colOrange
        # 0xffcfd8dc colSilver
        # 0xff546e7a colDarkGrey
        # 0xff29b6f6 colVelocity?
        # 0xff26a69a colKeyTrack?
        
        c = "colCyan"
        if "e53935" in color_hex.lower(): c = "colRed"
        elif "43a047" in color_hex.lower(): c = "colGreen"
        elif "ab47bc" in color_hex.lower(): c = "colPurple"
        elif "ffb300" in color_hex.lower(): c = "colAmber"
        elif "1e88e5" in color_hex.lower(): c = "colBlue"
        elif "fb8c00" in color_hex.lower(): c = "colOrange"
        elif "cfd8dc" in color_hex.lower(): c = "colSilver"
        elif "546e7a" in color_hex.lower(): c = "colDarkGrey"
        elif "29b6f6" in color_hex.lower(): c = "colCyan" # velocity
        elif "26a69a" in color_hex.lower(): c = "colTeal" # key track
        elif "d81b60" in color_hex.lower(): c = "colMagenta"
        
        params = []
        
        # Find setupKnob
        knobs = re.findall(r'setupKnob\(([a-zA-Z0-9_]+)', block)
        # Find selectors
        boxes = re.findall(r'bindSelector\([a-zA-Z0-9_]+,\s*[a-zA-Z0-9_]+,\s*"([^"]+)"', block)
        
        # Convert C++ slider names to param IDs (rough)
        # e.g., carrier1PitchSlider -> carrier1_pitch
        def to_param(name):
            name = name.replace('Slider', '')
            # add some hardcoded maps
            map_ = {
                'carrier1Pitch': 'carrier1_pitch', 'carrier1Shape': 'carrier1_shape', 'carrier1Depth': 'carrier1_depth',
                'mod1Shape': 'mod1_shape', 'mod1Ratio': 'mod1_ratio', 'mod1Depth': 'mod1_depth', 'mod1Feedback': 'mod1_feedback',
                'pitchEnv1Slope': 'pitch_env1_slope', 'pitchEnv1Depth': 'pitch_env1_depth', 'pitchEnv1Decay': 'pitch_env1_decay',
                'filter1Cutoff': 'filter1_cutoff', 'filter1Reso': 'filter1_res', 'filter1Drive': 'filter1_drive',
                'filterEnv1Slope': 'filter_env1_slope', 'filterEnv1Depth': 'filter_env1_depth', 'filterEnv1Decay': 'filter_env1_decay',
                'carrier2Pitch': 'carrier2_pitch', 'carrier2Shape': 'carrier2_shape', 'carrier2Depth': 'carrier2_depth',
                'mod2Shape': 'mod2_shape', 'mod2Ratio': 'mod2_ratio', 'mod2Depth': 'mod2_depth', 'mod2Feedback': 'mod2_feedback',
                'pitchEnv2Slope': 'pitch_env2_slope', 'pitchEnv2Depth': 'pitch_env2_depth', 'pitchEnv2Decay': 'pitch_env2_decay',
                'filter2Cutoff': 'filter2_cutoff', 'filter2Reso': 'filter2_res', 'filter2Drive': 'filter2_drive',
                'filterEnv2Slope': 'filter_env2_slope', 'filterEnv2Depth': 'filter_env2_depth', 'filterEnv2Decay': 'filter_env2_decay',
                'noiseShRate': 'noise_rate', 'noiseFilter': 'noise_color', 'noiseDecay': 'noise_decay', 'noiseCrossfade': 'noise_crossfade',
                'filter3Cutoff': 'filter3_cutoff', 'filter3Reso': 'filter3_res', 'filter3Drive': 'filter3_drive',
                'filterEnv3Slope': 'filter_env3_slope', 'filterEnv3Depth': 'filter_env3_depth', 'filterEnv3Decay': 'filter_env3_decay',
                'mixerFm1': 'mix_fm1', 'mixerFm2': 'mix_fm2', 'mixerNoise': 'mix_noise', 'mixerSub': 'mix_sub',
                'ampDrive': 'amp_drive', 'ampPan': 'amp_pan', 'ampVelSlope': 'amp_vel_slope', 'ampVelFloor': 'amp_vel_floor', 'ampLevel': 'amp_level',
                'ampEnvClaps': 'amp_env_claps', 'ampEnvClapSpeed': 'amp_env_clap_speed', 'ampEnvSlope': 'amp_env_slope', 'ampEnvDecay': 'amp_env_decay',
                'preLimiterGain': 'pre_limiter_gain', 'postLimiterGain': 'post_limiter_gain',
                'velSlope': 'vel_slope', 'velDepth': 'vel_depth',
                'keySlope': 'key_slope', 'keyDepth': 'key_depth',
                'slopFreq': 'slop_freq', 'slopDepth': 'slop_depth', 'slopDecay': 'slop_decay'
            }
            return map_.get(name, name)
            
        for b in boxes:
            params.append(b)
            
        for k in knobs:
            params.append(to_param(k))
            
        cards[title] = {
            "color": c,
            "style": style,
            "parameters": params
        }
    return cards

import json
tkf = parse_cpp('C:/Dev/TheKlangFarmer/source/FarmerEditor.cpp')
with open('C:/Dev/TheKlangFarmer/assets/layouts/tkf_layout.json', 'w', encoding='utf-8') as f:
    json.dump(tkf, f, indent=2)
