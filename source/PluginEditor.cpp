#include "PluginEditor.h"

// --- SAFE PARSING & FORMATTING HELPERS ---

static double parseNumberSafe(const juce::String& text, double fallback) {
    auto trimmed = text.trim();
    if (trimmed.isEmpty()) return fallback;

    // Check for note names like C4, A1, F#2, Bb3
    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int oct = -2; oct <= 8; ++oct) {
        for (int semi = 0; semi < 12; ++semi) {
            juce::String noteStr = noteNames[semi] + juce::String(oct);
            if (trimmed.startsWithIgnoreCase(noteStr)) {
                int noteNum = (oct + 1) * 12 + semi;
                return static_cast<double>(std::clamp(noteNum, 0, 127));
            }
        }
    }

    // Bracketed note number like [33]
    int bStart = trimmed.indexOfChar('[');
    int bEnd = trimmed.indexOfChar(']');
    if (bStart >= 0 && bEnd > bStart) {
        auto inside = trimmed.substring(bStart + 1, bEnd).trim();
        if (inside.isNotEmpty()) {
            double v = inside.getDoubleValue();
            if (std::isfinite(v)) return v;
        }
    }

    bool isKhz = trimmed.containsIgnoreCase("k");

    juce::String numStr;
    bool hasDot = false;
    for (int i = 0; i < trimmed.length(); ++i) {
        juce::juce_wchar ch = trimmed[i];
        if (std::isdigit(ch)) {
            numStr += ch;
        } else if (ch == '-' || ch == '+') {
            if (numStr.isEmpty()) numStr += ch;
        } else if (ch == '.' && !hasDot) {
            numStr += ch;
            hasDot = true;
        } else if (!numStr.isEmpty() && !std::isspace(ch)) {
            break;
        }
    }

    if (numStr.isEmpty()) return fallback;
    double val = numStr.getDoubleValue();
    if (!std::isfinite(val)) return fallback;

    if (isKhz && std::abs(val) < 1000.0) {
        val *= 1000.0;
    }

    return val;
}

static juce::String getMidiNoteName(int noteNumber) {
    noteNumber = std::clamp(noteNumber, 0, 127);
    const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (noteNumber / 12) - 1;
    return juce::String(names[noteNumber % 12]) + juce::String(octave) + " [" + juce::String(noteNumber) + "]";
}

// Formatters and parsers
static juce::String formatMidiNote(double val) {
    int note = static_cast<int>(std::round(val * 127.0));
    return getMidiNoteName(note);
}
static double parseMidiNote(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 36.0) / 127.0, 0.0, 1.0);
}

static juce::String formatSemi(double val) {
    float semi = static_cast<float>((val - 0.5) * 48.0);
    return juce::String(semi, 1) + " st";
}
static double parseSemi(const juce::String& text) {
    double semi = parseNumberSafe(text, 0.0);
    return std::clamp((semi / 48.0) + 0.5, 0.0, 1.0);
}

static juce::String formatSemi24(double val) {
    int semi = static_cast<int>(std::round((val - 0.5) * 48.0));
    return (semi > 0 ? "+" : "") + juce::String(semi) + " st";
}
static double parseSemi24(const juce::String& text) {
    double semi = parseNumberSafe(text, 0.0);
    return std::clamp((semi / 48.0) + 0.5, 0.0, 1.0);
}

static juce::String formatCarrierFreqHz(double val) {
    float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    if (hz >= 100.0f) return juce::String(hz, 1) + " Hz";
    return juce::String(hz, 2) + " Hz";
}
static double parseCarrierFreqHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 55.0);
    hz = std::clamp(hz, 20.0, 24000.0);
    return std::log(hz / 20.0) / std::log(24000.0 / 20.0);
}

static juce::String formatNoteDetail(double val) {
    int note = std::clamp(static_cast<int>(std::round(val * 127.0)), 0, 127);
    const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (note / 12) - 1;
    juce::String noteName = juce::String(names[note % 12]) + juce::String(octave);

    float f = 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
    juce::String hzStr;
    if (f >= 1000.0f) {
        hzStr = juce::String(f / 1000.0f, 1) + " kHz";
    } else {
        int hz = static_cast<int>(std::round(f));
        hzStr = juce::String(hz) + " Hz";
    }

    juce::String noteNumStr = juce::String(note);
    return noteName.paddedRight(' ', 4) + " [" + hzStr.paddedLeft(' ', 8) + ", " + noteNumStr.paddedLeft(' ', 3) + "]";
}

static double parseNoteDetail(const juce::String& text) {
    juce::String t = text.trim();
    if (t.isEmpty()) return 33.0 / 127.0;

    // Check if bracketed format e.g. "A1 [55 Hz, 33]"
    if (t.contains(",")) {
        juce::String afterComma = t.fromFirstOccurrenceOf(",", false, false).replaceCharacters("]", " ").trim();
        int noteNum = afterComma.getIntValue();
        if (noteNum >= 0 && noteNum <= 127) {
            return static_cast<double>(noteNum) / 127.0;
        }
    }

    // Try parsing as standard note name, e.g. "A1", "C#4", "Db2"
    juce::String notePart = t.upToFirstOccurrenceOf(" ", false, false).upToFirstOccurrenceOf("[", false, false).trim();
    if (notePart.isNotEmpty() && ((notePart[0] >= 'A' && notePart[0] <= 'G') || (notePart[0] >= 'a' && notePart[0] <= 'g'))) {
        char base = static_cast<char>(std::toupper(notePart[0]));
        int semitone = 0;
        switch (base) {
            case 'C': semitone = 0; break;
            case 'D': semitone = 2; break;
            case 'E': semitone = 4; break;
            case 'F': semitone = 5; break;
            case 'G': semitone = 7; break;
            case 'A': semitone = 9; break;
            case 'B': semitone = 11; break;
            default: break;
        }
        int idx = 1;
        if (notePart.length() > idx && (notePart[idx] == '#' || notePart[idx] == 's' || notePart[idx] == 'S')) {
            semitone += 1;
            idx++;
        } else if (notePart.length() > idx && (notePart[idx] == 'b' || notePart[idx] == 'B')) {
            semitone -= 1;
            idx++;
        }
        juce::String octStr = notePart.substring(idx).trim();
        if (octStr.isNotEmpty()) {
            int oct = octStr.getIntValue();
            int midi = (oct + 1) * 12 + semitone;
            return std::clamp(static_cast<double>(midi) / 127.0, 0.0, 1.0);
        }
    }

    // Try parsing as Hz if contains "hz"
    if (t.containsIgnoreCase("hz")) {
        double hz = parseNumberSafe(t, 55.0);
        hz = std::clamp(hz, 8.0, 24000.0);
        double midi = 69.0 + 12.0 * std::log2(hz / 440.0);
        return std::clamp(std::round(midi) / 127.0, 0.0, 1.0);
    }

    // Otherwise try parsing as raw MIDI number (e.g. 33)
    double raw = parseNumberSafe(t, 33.0);
    return std::clamp(std::round(raw) / 127.0, 0.0, 1.0);
}

static juce::String formatRatio(double val) {
    float r = static_cast<float>(val) * 16.0f;
    return juce::String(r, 2) + "x";
}
static double parseRatio(const juce::String& text) {
    double r = parseNumberSafe(text, 1.0);
    return std::clamp(r / 16.0, 0.0, 1.0);
}

static juce::String formatPercent(double val) {
    return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
}
static double parsePercent(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 50.0) / 100.0, 0.0, 1.0);
}

static juce::String formatBipolarPercent(double val) {
    int p = static_cast<int>(std::round((val - 0.5) * 200.0));
    return (p > 0 ? "+" : "") + juce::String(p) + "%";
}
static double parseBipolarPercent(const juce::String& text) {
    double p = parseNumberSafe(text, 0.0);
    return std::clamp((p / 200.0) + 0.5, 0.0, 1.0);
}

static juce::String formatTimeMs(double val) {
    float ms = 5.0f * std::pow(60000.0f / 5.0f, static_cast<float>(val));
    if (ms >= 1000.0f) return juce::String(ms / 1000.0f, 2) + " s";
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
static double parseTimeMs(const juce::String& text) {
    double ms = parseNumberSafe(text, 333.0);
    if (text.containsIgnoreCase("s") && !text.containsIgnoreCase("ms")) ms *= 1000.0;
    ms = std::clamp(ms, 5.0, 60000.0);
    return std::log(ms / 5.0) / std::log(60000.0 / 5.0);
}

static juce::String formatNoiseTimeMs(double val) {
    float ms = 1.0f * std::pow(60000.0f / 1.0f, static_cast<float>(val));
    if (ms >= 1000.0f) return juce::String(ms / 1000.0f, 2) + " s";
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
static double parseNoiseTimeMs(const juce::String& text) {
    double ms = parseNumberSafe(text, 100.0);
    if (text.containsIgnoreCase("s") && !text.containsIgnoreCase("ms")) ms *= 1000.0;
    ms = std::clamp(ms, 1.0, 60000.0);
    return std::log(ms / 1.0) / std::log(60000.0 / 1.0);
}

static juce::String formatFreqHz(double val) {
    float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    if (hz >= 10.0f) return juce::String(hz, 1) + " Hz";
    return juce::String(hz, 2) + " Hz";
}
static double parseFreqHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 1000.0);
    hz = std::clamp(hz, 0.1, 24000.0);
    return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
}

static juce::String formatEqFreqHz(double val) {
    float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    return juce::String(static_cast<int>(std::round(hz))) + " Hz";
}
static double parseEqFreqHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 1000.0);
    hz = std::clamp(hz, 20.0, 24000.0);
    return std::log(hz / 20.0) / std::log(24000.0 / 20.0);
}

static juce::String formatDb(double val) {
    float db = -6.0f + static_cast<float>(val) * 30.0f;
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
static double parseDb(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 6.0) / 30.0, 0.0, 1.0);
}

static juce::String formatBipolarDb(double val) {
    float db = static_cast<float>((val - 0.5) * 48.0);
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
static double parseBipolarDb(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db / 48.0) + 0.5, 0.0, 1.0);
}

static juce::String formatWetDry(double val) {
    double b = (val - 0.5) * 2.0;
    int wet = static_cast<int>(std::round(std::abs(b) * 100.0));
    int dry = static_cast<int>(std::round((1.0 - std::abs(b)) * 100.0));
    if (b > 0.005) {
        return "+" + juce::String(wet) + "%:" + juce::String(dry) + "%";
    } else if (b < -0.005) {
        return "-" + juce::String(wet) + "%:" + juce::String(dry) + "%";
    } else {
        return juce::String("0%:100%");
    }
}

static double parseWetDry(const juce::String& text) {
    auto t = text.trim();
    if (t.containsChar(':')) {
        auto parts = juce::StringArray::fromTokens(t, ":", "");
        if (parts.size() >= 1) {
            double sign = t.startsWith("-") ? -1.0 : 1.0;
            double wet = parseNumberSafe(parts[0], 50.0);
            return std::clamp((sign * std::abs(wet) / 200.0) + 0.5, 0.0, 1.0);
        }
    }
    return parseBipolarPercent(text);
}

static juce::String formatMixerLevel(double val) {
    float pct = (val <= 0.5) ? static_cast<float>(val * 200.0) : static_cast<float>(100.0 + (val - 0.5) * 600.0);
    return juce::String(static_cast<int>(std::round(pct))) + "%";
}
static double parseMixerLevel(const juce::String& text) {
    double pct = parseNumberSafe(text, 100.0);
    pct = std::clamp(pct, 0.0, 400.0);
    if (pct <= 100.0) return pct / 200.0;
    return 0.5 + (pct - 100.0) / 600.0;
}

static juce::String formatOctaves(double val) {
    float oct = static_cast<float>((val - 0.5) * 10.0);
    return (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct";
}
static double parseOctaves(const juce::String& text) {
    double oct = parseNumberSafe(text, 0.0);
    return std::clamp((oct / 10.0) + 0.5, 0.0, 1.0);
}

static juce::String formatFilterOctaves(double val) {
    float oct = static_cast<float>((val - 0.5) * 20.0);
    return (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct";
}
static double parseFilterOctaves(const juce::String& text) {
    double oct = parseNumberSafe(text, 0.0);
    return std::clamp((oct / 20.0) + 0.5, 0.0, 1.0);
}

static juce::String formatBits(double val) {
    float b = 1.0f + static_cast<float>(val) * 15.0f;
    return juce::String(b, 1) + " bit";
}
static double parseBits(const juce::String& text) {
    double b = parseNumberSafe(text, 16.0);
    return std::clamp((b - 1.0) / 15.0, 0.0, 1.0);
}

static juce::String formatWavefolds(double val) {
    float f = static_cast<float>(val) * 8.0f;
    return juce::String(f, 2);
}
static double parseWavefolds(const juce::String& text) {
    double f = parseNumberSafe(text, 0.0);
    return std::clamp(f / 8.0, 0.0, 1.0);
}

static juce::String formatStages(double val) {
    int s = static_cast<int>(std::round(val * 32.0));
    return juce::String(s);
}
static double parseStages(const juce::String& text) {
    double s = parseNumberSafe(text, 4.0);
    return std::clamp(s / 32.0, 0.0, 1.0);
}

static juce::String formatClaps(double val) {
    int c = static_cast<int>(std::round(val * 32.0));
    return juce::String(c);
}
static double parseClaps(const juce::String& text) {
    double c = parseNumberSafe(text, 0.0);
    return std::clamp(c / 32.0, 0.0, 1.0);
}

static juce::String formatClapSpeed(double val) {
    float ms = 1.0f + static_cast<float>(val) * 14.0f;
    return juce::String(ms, 1) + " ms";
}
static double parseClapSpeed(const juce::String& text) {
    double ms = parseNumberSafe(text, 3.0);
    return std::clamp((ms - 1.0) / 14.0, 0.0, 1.0);
}

static juce::String formatLimiterGain(double val) {
    float db = -12.0f + static_cast<float>(val) * 36.0f;
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
static double parseLimiterGain(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 12.0) / 36.0, 0.0, 1.0);
}

static juce::String formatLimiterThresh(double val) {
    float db = -24.0f + static_cast<float>(val) * 24.0f;
    return juce::String(db, 1) + " dB";
}
static double parseLimiterThresh(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 24.0) / 24.0, 0.0, 1.0);
}

static juce::String formatLimiterRelease(double val) {
    float ms = 1.0f * std::pow(500.0f / 1.0f, static_cast<float>(val));
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
static double parseLimiterRelease(const juce::String& text) {
    double ms = parseNumberSafe(text, 50.0);
    ms = std::clamp(ms, 1.0, 500.0);
    return std::log(ms / 1.0) / std::log(500.0 / 1.0);
}

static juce::String formatSlop(double val) {
    return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
}
static double parseSlop(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 0.0) / 100.0, 0.0, 1.0);
}

static juce::String formatVelocitySlope(double val) {
    if (val < 0.70) return "EXP";
    if (val > 0.80) return "LOG";
    return "LIN";
}
static double parseVelocitySlope(const juce::String& text) {
    if (text.containsIgnoreCase("exp")) return 0.55125;
    if (text.containsIgnoreCase("log")) return 1.0;
    return 0.75;
}

static juce::String formatSlope(double val) {
    if (val < 0.70) return "EXP";
    if (val > 0.80) return "LOG";
    return "LIN";
}
static double parseSlope(const juce::String& text) {
    if (text.containsIgnoreCase("exp")) return 0.55125;
    if (text.containsIgnoreCase("log")) return 1.0;
    return 0.75;
}

static juce::String formatEqWidthOct(double val) {
    float oct = 0.1f * std::pow(100.0f, static_cast<float>(val));
    return juce::String(oct, 2) + " oct";
}
static double parseEqWidthOct(const juce::String& text) {
    double oct = parseNumberSafe(text, 0.1);
    oct = std::clamp(oct, 0.1, 10.0);
    return std::log(oct / 0.1) / std::log(100.0);
}

static juce::String formatRangeHz(double val) {
    float hz = TbdAudio::normToRangeHz(static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    return juce::String(hz, 1) + " Hz";
}
static double parseRangeHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 3.0);
    return static_cast<double>(TbdAudio::rangeHzToNorm(static_cast<float>(hz)));
}

static juce::String formatChorusRate(double val) {
    float hz = 0.1f * std::pow(100.0f, static_cast<float>(val));
    return juce::String(hz, (hz < 1.0f ? 2 : 1)) + " Hz";
}
static double parseChorusRate(const juce::String& text) {
    double hz = parseNumberSafe(text, 1.2);
    hz = std::clamp(hz, 0.1, 10.0);
    return std::clamp(std::log(hz / 0.1) / std::log(100.0), 0.0, 1.0);
}

static juce::String formatPhaserRate(double val) {
    float hz = 0.05f * std::pow(160.0f, static_cast<float>(val));
    return juce::String(hz, (hz < 1.0f ? 2 : 1)) + " Hz";
}
static double parsePhaserRate(const juce::String& text) {
    double hz = parseNumberSafe(text, 0.5);
    hz = std::clamp(hz, 0.05, 8.0);
    return std::clamp(std::log(hz / 0.05) / std::log(160.0), 0.0, 1.0);
}

static juce::String formatFlangerRate(double val) {
    float hz = 0.05f * std::pow(100.0f, static_cast<float>(val));
    return juce::String(hz, (hz < 1.0f ? 2 : 1)) + " Hz";
}
static double parseFlangerRate(const juce::String& text) {
    double hz = parseNumberSafe(text, 0.25);
    hz = std::clamp(hz, 0.05, 5.0);
    return std::clamp(std::log(hz / 0.05) / std::log(100.0), 0.0, 1.0);
}

static juce::String formatDelayDiv(double val) {
    const juce::String names[] = { "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4", "1/4D", "1/2" };
    int idx = std::clamp(static_cast<int>(std::round(val * 9.0)), 0, 9);
    return names[idx];
}
static double parseDelayDiv(const juce::String& text) {
    const juce::String names[] = { "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4", "1/4D", "1/2" };
    for (int i = 0; i < 10; ++i) {
        if (text.trim().equalsIgnoreCase(names[i])) return static_cast<double>(i) / 9.0;
    }
    return 5.0 / 9.0;
}

static juce::String formatDelayTone(double val) {
    float hz = 500.0f * std::pow(40.0f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 1) + " kHz";
    return juce::String(static_cast<int>(std::round(hz))) + " Hz";
}
static double parseDelayTone(const juce::String& text) {
    auto t = text.trim().toLowerCase();
    double mult = 1.0;
    if (t.endsWith("khz") || t.endsWith("k")) mult = 1000.0;
    double hz = parseNumberSafe(text, 8000.0) * mult;
    hz = std::clamp(hz, 500.0, 20000.0);
    return std::clamp(std::log(hz / 500.0) / std::log(40.0), 0.0, 1.0);
}


// --- ROTARY KNOB LOOK AND FEEL ---

RotaryKnobLookAndFeel::RotaryKnobLookAndFeel() {
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff232733));
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff161922));
    setColour(juce::Slider::trackColourId, juce::Colour(0xff00d2ff));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xffe8edf5));
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe8edf5));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff12141a));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2c3240));

    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff14161d));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2c3240));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8edf5));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff8b95a8));

    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1a1d26));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xffe8edf5));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff00d2ff));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xff0f1115));
}

juce::Label* RotaryKnobLookAndFeel::createSliderTextBox(juce::Slider& slider) {
    if (auto* rks = dynamic_cast<RotaryKnobSlider*>(&slider)) {
        return new DiagramSliderLabel(*rks);
    }
    auto* l = juce::LookAndFeel_V4::createSliderTextBox(slider);
    l->setFont(juce::FontOptions(13.5f, juce::Font::bold));
    l->setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
    l->setColour(juce::Label::backgroundColourId, juce::Colour(0xee11141a));
    l->setColour(juce::Label::outlineColourId, juce::Colour(0x44303848));
    l->setJustificationType(juce::Justification::centred);
    return l;
}

juce::Font RotaryKnobLookAndFeel::getTextButtonFont(juce::TextButton&, int) {
    return juce::Font(juce::FontOptions(13.5f, juce::Font::bold));
}

juce::Font RotaryKnobLookAndFeel::getComboBoxFont(juce::ComboBox&) {
    return juce::Font(juce::FontOptions(13.0f, juce::Font::bold));
}

juce::Font RotaryKnobLookAndFeel::getPopupMenuFont() {
    return juce::Font(juce::FontOptions(13.0f, juce::Font::bold));
}

void RotaryKnobLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float minSliderPos, float maxSliderPos,
                                             const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal) {
        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    auto fillColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
    bool isBipolar = false;
    if (auto* rks = dynamic_cast<RotaryKnobSlider*>(&slider)) {
        isBipolar = rks->isBipolar;
    }

    float trackHeight = 6.0f;
    float trackY = static_cast<float>(y) + (static_cast<float>(height) - trackHeight) * 0.5f;
    float trackX = static_cast<float>(x) + 2.0f;
    float trackW = static_cast<float>(width) - 4.0f;
    juce::Rectangle<float> trackRect(trackX, trackY, trackW, trackHeight);

    g.setColour(juce::Colour(0xff161922));
    g.fillRoundedRectangle(trackRect, 3.0f);
    g.setColour(juce::Colour(0xff2a3040));
    g.drawRoundedRectangle(trackRect, 3.0f, 1.0f);

    if (slider.isEnabled() && trackW > 2.0f) {
        float minX = trackX;
        float maxX = trackX + trackW;
        float clampedPos = std::clamp(sliderPos, minX, maxX);

        if (isBipolar) {
            float midX = trackX + trackW * 0.5f;
            float startX = std::min(midX, clampedPos);
            float endX = std::max(midX, clampedPos);
            float fillW = endX - startX;

            if (fillW > 0.5f) {
                juce::Rectangle<float> fillRect(startX, trackY + 1.0f, fillW, trackHeight - 2.0f);
                g.setColour(fillColour.withAlpha(0.85f));
                g.fillRoundedRectangle(fillRect, 2.0f);
            }
            g.setColour(juce::Colour(0xff5a667d));
            g.drawVerticalLine(static_cast<int>(midX), trackY - 2.0f, trackY + trackHeight + 2.0f);
        } else {
            float fillW = clampedPos - minX;
            if (fillW > 0.5f) {
                juce::Rectangle<float> fillRect(minX + 1.0f, trackY + 1.0f, fillW - 1.0f, trackHeight - 2.0f);
                g.setColour(fillColour.withAlpha(0.85f));
                g.fillRoundedRectangle(fillRect, 2.0f);
            }
        }
    }

    float thumbW = 7.0f;
    float thumbH = std::min(static_cast<float>(height) - 2.0f, 16.0f);
    float thumbY = static_cast<float>(y) + (static_cast<float>(height) - thumbH) * 0.5f;
    float thumbX = std::clamp(sliderPos - thumbW * 0.5f, static_cast<float>(x), static_cast<float>(x + width) - thumbW);

    juce::Rectangle<float> thumbRect(thumbX, thumbY, thumbW, thumbH);

    if (slider.isMouseOverOrDragging()) {
        g.setColour(fillColour.withAlpha(0.35f));
        g.drawRoundedRectangle(thumbRect.expanded(1.5f), 2.5f, 1.5f);
    }

    juce::ColourGradient thumbGrad(juce::Colour(0xffeff3fa), thumbX, thumbY,
                                  juce::Colour(0xffb0bac9), thumbX, thumbY + thumbH, false);
    g.setGradientFill(thumbGrad);
    g.fillRoundedRectangle(thumbRect, 2.0f);
    g.setColour(juce::Colour(0xff12151c));
    g.drawRoundedRectangle(thumbRect, 2.0f, 1.0f);
    g.setColour(fillColour);
    g.drawVerticalLine(static_cast<int>(thumbRect.getCentreX()), thumbY + 2.5f, thumbY + thumbH - 2.5f);
}

void RotaryKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    auto radius = juce::jlimit(11.0f, 24.0f, juce::jmin(bounds.getWidth() * 0.46f, bounds.getHeight() * 0.46f));
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto centre = bounds.getCentre();
    auto lineW = juce::jmax(2.4f, radius * 0.18f);

    auto arcRadius = radius - lineW * 0.5f;
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    bool isBipolar = false;
    if (auto* rks = dynamic_cast<RotaryKnobSlider*>(&slider)) {
        isBipolar = rks->isBipolar;
    }

    if (slider.isEnabled()) {
        juce::Path valueArc;
        if (isBipolar) {
            float midAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
            if (std::abs(toAngle - midAngle) > 0.005f) {
                if (toAngle >= midAngle) {
                    valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                           0.0f, midAngle, toAngle, true);
                } else {
                    valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                           0.0f, toAngle, midAngle, true);
                }
                g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
                g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        } else {
            if (sliderPosProportional > 0.001f) {
                valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                       0.0f, rotaryStartAngle, toAngle, true);
                g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
                g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
    }

    auto innerRadius = arcRadius - lineW * 0.85f;
    if (innerRadius > 3.0f) {
        auto knobBounds = juce::Rectangle<float>(centre.x - innerRadius, centre.y - innerRadius,
                                                 innerRadius * 2.0f, innerRadius * 2.0f);
        juce::ColourGradient grad(juce::Colour(0xff2a2f3d), centre.x, centre.y - innerRadius,
                                  juce::Colour(0xff14161c), centre.x, centre.y + innerRadius, false);
        g.setGradientFill(grad);
        g.fillEllipse(knobBounds);
        g.setColour(juce::Colour(0xff3b4354));
        g.drawEllipse(knobBounds, 1.0f);

        if (slider.isMouseOverOrDragging()) {
            auto fillCol = slider.findColour(juce::Slider::rotarySliderFillColourId);
            g.setColour(fillCol.withAlpha(0.30f));
            g.drawEllipse(knobBounds.expanded(1.5f), 1.0f);
        }

        juce::Path p;
        auto pointerLength = innerRadius * 0.68f;
        auto pointerThickness = juce::jmax(1.8f, innerRadius * 0.16f);
        p.addRoundedRectangle(-pointerThickness * 0.5f, -innerRadius + 1.0f, pointerThickness, pointerLength, 1.0f);
        p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
        g.setColour(juce::Colour(0xffffffff));
        g.fillPath(p);
    }
}

// --- MINI OSCILLOSCOPE COMPONENT ---

MiniOscilloscopeComponent::MiniOscilloscopeComponent(juce::Colour traceColour)
    : traceCol(traceColour)
{
    setOpaque(false);
}

void MiniOscilloscopeComponent::updateData(const float* data, int numPoints) {
    if (!data || numPoints <= 0) return;
    points.assign(data, data + numPoints);
    repaint();
}

void MiniOscilloscopeComponent::setPlotMode(PlotMode mode) {
    if (plotMode != mode) {
        plotMode = mode;
        repaint();
    }
}

void MiniOscilloscopeComponent::updateFilterParams(int type, int slope, float cutoffHz, float resonance) {
    if (filterType != type || filterSlope != slope || filterCutoff != cutoffHz || filterResonance != resonance) {
        filterType = type;
        filterSlope = slope;
        filterCutoff = cutoffHz;
        filterResonance = resonance;
        repaint();
    }
}

void MiniOscilloscopeComponent::updateEqParams(float freqHz, float widthOct, float gainDb, float djFilter) {
    if (eqFreq != freqHz || eqWidth != widthOct || eqGain != gainDb || eqDJ != djFilter) {
        eqFreq = freqHz;
        eqWidth = widthOct;
        eqGain = gainDb;
        eqDJ = djFilter;
        repaint();
    }
}

void MiniOscilloscopeComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff0e1017));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(0xff1d222e));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    if (plotMode == PlotMode::Oscilloscope) {
        float midY = bounds.getCentreY();
        g.setColour(juce::Colour(0xff1b202c));
        g.drawHorizontalLine(static_cast<int>(midY), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);

        if (points.empty()) return;

        juce::Path p;
        float dx = bounds.getWidth() / static_cast<float>(points.size() - 1);
        float halfH = (bounds.getHeight() - 4.0f) * 0.5f;

        for (size_t i = 0; i < points.size(); ++i) {
            float val = std::clamp(points[i], -1.5f, 1.5f);
            float y = midY - (val * (halfH / 1.15f));
            y = std::clamp(y, bounds.getY() + 1.0f, bounds.getBottom() - 1.0f);
            float x = bounds.getX() + static_cast<float>(i) * dx;

            if (i == 0) p.startNewSubPath(x, y);
            else p.lineTo(x, y);
        }

        g.setColour(traceCol.withAlpha(0.25f));
        g.strokePath(p, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(traceCol);
        g.strokePath(p, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        return;
    }

    float width = bounds.getWidth();
    float height = bounds.getHeight();
    float left = bounds.getX();
    float top = bounds.getY();
    float right = bounds.getRight();
    float bottom = bounds.getBottom();

    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 24000.0f;
    float logRange = std::log(maxFreq / minFreq);

    auto freqToX = [&](float f) -> float {
        float norm = std::log(std::clamp(f, minFreq, maxFreq) / minFreq) / logRange;
        return left + norm * width;
    };

    g.setColour(juce::Colour(0xff181c26));
    float x100 = freqToX(100.0f);
    float x1k  = freqToX(1000.0f);
    float x10k = freqToX(10000.0f);
    g.drawVerticalLine(static_cast<int>(x100), top + 2.0f, bottom - 2.0f);
    g.drawVerticalLine(static_cast<int>(x1k),  top + 2.0f, bottom - 2.0f);
    g.drawVerticalLine(static_cast<int>(x10k), top + 2.0f, bottom - 2.0f);

    g.setColour(juce::Colour(0xff5a667d));
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.drawText("100", static_cast<int>(x100) - 14, static_cast<int>(bottom) - 15, 28, 14, juce::Justification::centred);
    g.drawText("1k",  static_cast<int>(x1k) - 12,   static_cast<int>(bottom) - 15, 24, 14, juce::Justification::centred);
    g.drawText("10k", static_cast<int>(x10k) - 14, static_cast<int>(bottom) - 15, 28, 14, juce::Justification::centred);

    juce::Path curvePath;
    float yZero = top + height * 0.5f;

    if (plotMode == PlotMode::FilterXY) {
        yZero = top + height * 0.35f;
        g.setColour(juce::Colour(0xff222838));
        g.drawHorizontalLine(static_cast<int>(yZero), left + 2.0f, right - 2.0f);

        float yPlus12 = yZero - (12.0f / 36.0f) * (height * 0.55f);
        g.setColour(juce::Colour(0xff161a24));
        g.drawHorizontalLine(static_cast<int>(yPlus12), left + 2.0f, right - 2.0f);

        int numSteps = std::max(64, static_cast<int>(width));
        float fc = std::clamp(filterCutoff, 0.1f, 24000.0f);
        float qVal = 0.707f + filterResonance * 18.0f;
        if (filterSlope == 0) qVal = 0.5f + filterResonance * 5.0f;

        for (int i = 0; i < numSteps; ++i) {
            float u = static_cast<float>(i) / static_cast<float>(numSteps - 1);
            float f = minFreq * std::pow(maxFreq / minFreq, u);
            float px = left + u * width;

            float r = f / fc;
            float d = std::sqrt((1.0f - r * r) * (1.0f - r * r) + (r / qVal) * (r / qVal));
            float mag = 1.0f;

            if (filterType == 0) { // LPF
                if (filterSlope == 0)      mag = 1.0f / std::sqrt(1.0f + r * r);
                else if (filterSlope == 1) mag = 1.0f / std::max(d, 1e-4f);
                else if (filterSlope == 2) mag = (1.0f / std::max(d, 1e-4f)) * (1.0f / std::sqrt(1.0f + r * r));
                else if (filterSlope == 3) mag = (1.0f / std::max(d, 1e-4f)) * (1.0f / std::max(d, 1e-4f));
                else                       mag = std::pow(1.0f / std::max(d, 1e-4f), 3.0f);
            } else if (filterType == 1) { // BPF
                float bMag = (r / qVal) / std::max(d, 1e-4f);
                if (filterSlope <= 1) mag = bMag;
                else                  mag = bMag * (1.0f / std::sqrt(1.0f + r * r));
            } else if (filterType == 2) { // HPF
                if (filterSlope == 0)      mag = r / std::sqrt(1.0f + r * r);
                else if (filterSlope == 1) mag = (r * r) / std::max(d, 1e-4f);
                else if (filterSlope == 2) mag = ((r * r) / std::max(d, 1e-4f)) * (r / std::sqrt(1.0f + r * r));
                else if (filterSlope == 3) mag = ((r * r) / std::max(d, 1e-4f)) * ((r * r) / std::max(d, 1e-4f));
                else                       mag = std::pow((r * r) / std::max(d, 1e-4f), 3.0f);
            } else if (filterType == 3) { // BRF (Notch)
                mag = std::abs(1.0f - r * r) / std::max(d, 1e-4f);
            }
            float gainDb = 20.0f * std::log10(std::clamp(mag, 1e-4f, 16.0f));

            float py = yZero - (gainDb / 36.0f) * (height * 0.55f);
            py = std::clamp(py, top + 1.0f, bottom - 1.0f);

            if (i == 0) curvePath.startNewSubPath(px, py);
            else curvePath.lineTo(px, py);
        }

        float xCutoff = freqToX(fc);
        g.setColour(traceCol.withAlpha(0.2f));
        g.drawVerticalLine(static_cast<int>(xCutoff), top + 2.0f, bottom - 2.0f);
    } else if (plotMode == PlotMode::EqXY) {
        yZero = top + height * 0.5f;
        g.setColour(juce::Colour(0xff222838));
        g.drawHorizontalLine(static_cast<int>(yZero), left + 2.0f, right - 2.0f);

        float yPlus12  = yZero - (12.0f / 24.0f) * (height * 0.42f);
        float yMinus12 = yZero + (12.0f / 24.0f) * (height * 0.42f);
        g.setColour(juce::Colour(0xff161a24));
        g.drawHorizontalLine(static_cast<int>(yPlus12), left + 2.0f, right - 2.0f);
        g.drawHorizontalLine(static_cast<int>(yMinus12), left + 2.0f, right - 2.0f);

        int numSteps = std::max(64, static_cast<int>(width));
        float f0 = std::clamp(eqFreq, 20.0f, 24000.0f);
        float bw = std::clamp(eqWidth, 0.1f, 10.0f);

        for (int i = 0; i < numSteps; ++i) {
            float u = static_cast<float>(i) / static_cast<float>(numSteps - 1);
            float f = minFreq * std::pow(maxFreq / minFreq, u);
            float px = left + u * width;

            float octDist = std::log2(f / f0);
            float term = (2.0f * octDist) / bw;
            float bellDb = eqGain * (1.0f / (1.0f + term * term));

            float djDb = 0.0f;
            if (eqDJ < 0.49f) {
                float fCut = minFreq * std::pow(maxFreq / minFreq, eqDJ * 2.0f);
                djDb = 20.0f * std::log10(1.0f / std::sqrt(1.0f + (f / fCut) * (f / fCut)));
            } else if (eqDJ > 0.51f) {
                float fCut = minFreq * std::pow(maxFreq / minFreq, (eqDJ - 0.5f) * 2.0f);
                djDb = 20.0f * std::log10((f / fCut) / std::sqrt(1.0f + (f / fCut) * (f / fCut)));
            }

            float totalDb = bellDb + djDb;
            float py = yZero - (totalDb / 24.0f) * (height * 0.42f);
            py = std::clamp(py, top + 1.0f, bottom - 1.0f);

            if (i == 0) curvePath.startNewSubPath(px, py);
            else curvePath.lineTo(px, py);
        }

        float xPeak = freqToX(f0);
        float yPeak = yZero - (eqGain / 24.0f) * (height * 0.42f);
        yPeak = std::clamp(yPeak, top + 2.0f, bottom - 2.0f);
        g.setColour(traceCol.withAlpha(0.25f));
        g.drawVerticalLine(static_cast<int>(xPeak), top + 2.0f, bottom - 2.0f);
        g.setColour(traceCol);
        g.fillEllipse(xPeak - 2.5f, yPeak - 2.5f, 5.0f, 5.0f);
    }

    juce::Path fillPath(curvePath);
    fillPath.lineTo(right, yZero);
    fillPath.lineTo(left, yZero);
    fillPath.closeSubPath();
    g.setColour(traceCol.withAlpha(0.12f));
    g.fillPath(fillPath);

    g.setColour(traceCol.withAlpha(0.25f));
    g.strokePath(curvePath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(traceCol);
    g.strokePath(curvePath, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

// --- DIAGRAM SLIDER LABEL ---

DiagramSliderLabel::DiagramSliderLabel(RotaryKnobSlider& s)
    : slider(s)
{
    slider.addListener(this);
    setFont(juce::FontOptions(13.5f, juce::Font::bold));
    setJustificationType(juce::Justification::centred);
    setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
    setColour(juce::Label::backgroundColourId, juce::Colour(0xee11141a));
    setColour(juce::Label::outlineColourId, juce::Colour(0x44303848));
}

DiagramSliderLabel::~DiagramSliderLabel() {
    slider.removeListener(this);
}

void DiagramSliderLabel::paint(juce::Graphics& g) {
    if (slider.diagramType == RotaryKnobSlider::DiagramType::None) {
        juce::Label::paint(g);
        return;
    }

    auto bounds = getLocalBounds().toFloat().reduced(1.5f, 1.0f);
    g.setColour(juce::Colour(0xee11141a));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(juce::Colour(0x44303848));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    auto drawArea = bounds.reduced(5.0f, 2.5f);
    if (drawArea.getWidth() <= 4.0f || drawArea.getHeight() <= 4.0f) return;

    juce::Colour traceColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
    float val = static_cast<float>(slider.getValue());

    if (slider.diagramType == RotaryKnobSlider::DiagramType::Waveform) {
        g.setColour(juce::Colour(0x22ffffff));
        g.drawHorizontalLine(static_cast<int>(drawArea.getCentreY()), drawArea.getX(), drawArea.getRight());

        juce::Path p;
        constexpr int numPts = 32;
        for (int i = 0; i <= numPts; ++i) {
            float phase = static_cast<float>(i) / static_cast<float>(numPts);
            float waveY = TbdAudio::evaluateWaveform(phase, val);
            float px = drawArea.getX() + phase * drawArea.getWidth();
            float py = drawArea.getCentreY() - waveY * (drawArea.getHeight() * 0.44f);
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(traceColour);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (slider.diagramType == RotaryKnobSlider::DiagramType::EnvelopeSlope) {
        g.setColour(juce::Colour(0x22ffffff));
        g.drawHorizontalLine(static_cast<int>(drawArea.getBottom() - 1.0f), drawArea.getX(), drawArea.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float y = TbdAudio::applyEnvelopeSlope(1.0f - t, val);
            float px = drawArea.getX() + t * drawArea.getWidth();
            float py = drawArea.getBottom() - y * (drawArea.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(traceColour);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (slider.diagramType == RotaryKnobSlider::DiagramType::VelocitySlope) {
        g.setColour(juce::Colour(0x22ffffff));
        g.drawHorizontalLine(static_cast<int>(drawArea.getBottom() - 1.0f), drawArea.getX(), drawArea.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float y = TbdAudio::applyEnvelopeSlope(t, val);
            float px = drawArea.getX() + t * drawArea.getWidth();
            float py = drawArea.getBottom() - y * (drawArea.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(traceColour);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (slider.diagramType == RotaryKnobSlider::DiagramType::FilterSlope) {
        g.setColour(juce::Colour(0x22ffffff));
        g.drawHorizontalLine(static_cast<int>(drawArea.getBottom() - 1.0f), drawArea.getX(), drawArea.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        float cutoffX = 0.55f;
        float exponent = 1.0f + val * 6.0f;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float mag = 1.0f;
            if (t > cutoffX) {
                float f = (t - cutoffX) / (1.0f - cutoffX);
                mag = std::clamp(1.0f - std::pow(f, 1.0f / exponent), 0.0f, 1.0f);
            }
            float px = drawArea.getX() + t * drawArea.getWidth();
            float py = drawArea.getBottom() - mag * (drawArea.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(traceColour);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

void DiagramSliderLabel::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
        slider.openHoveringEditor();
        return;
    }
    if (slider.diagramType != RotaryKnobSlider::DiagramType::None) {
        slider.mouseDown(e.getEventRelativeTo(&slider));
        return;
    }
    juce::Label::mouseDown(e);
}

void DiagramSliderLabel::mouseDrag(const juce::MouseEvent& e) {
    if (slider.diagramType != RotaryKnobSlider::DiagramType::None) {
        slider.mouseDrag(e.getEventRelativeTo(&slider));
        return;
    }
    juce::Label::mouseDrag(e);
}

void DiagramSliderLabel::mouseUp(const juce::MouseEvent& e) {
    if (slider.diagramType != RotaryKnobSlider::DiagramType::None) {
        slider.mouseUp(e.getEventRelativeTo(&slider));
        return;
    }
    juce::Label::mouseUp(e);
}

void DiagramSliderLabel::mouseDoubleClick(const juce::MouseEvent& e) {
    slider.mouseDoubleClick(e);
}

void DiagramSliderLabel::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    slider.mouseWheelMove(e.getEventRelativeTo(&slider), wheel);
}

// --- ROTARY KNOB SLIDER (ARCADE HP METER) ---

RotaryKnobSlider::RotaryKnobSlider() {
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setScrollWheelEnabled(true);
}

void RotaryKnobSlider::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
        openHoveringEditor();
        return;
    }
    dragStartPos = e.getPosition();
    dragStartVal = getValue();
    startedDragging();
}

void RotaryKnobSlider::mouseDrag(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) return;

    int dx = e.getPosition().x - dragStartPos.x;
    int dy = dragStartPos.y - e.getPosition().y; // Upward dragging is positive

    float delta = (std::abs(dx) > std::abs(dy)) ? static_cast<float>(dx) : static_cast<float>(dy);
    float sensitivity = e.mods.isShiftDown() ? 0.001f : 0.005f;
    double range = getMaximum() - getMinimum();
    double targetVal = std::clamp(dragStartVal + static_cast<double>(delta * sensitivity) * range,
                                  getMinimum(), getMaximum());
    setValue(targetVal, juce::sendNotificationAsync);
}

void RotaryKnobSlider::mouseUp(const juce::MouseEvent&) {
    stoppedDragging();
}

void RotaryKnobSlider::mouseDoubleClick(const juce::MouseEvent&) {
    if (getDefaultValue) {
        setValue(getDefaultValue(), juce::sendNotificationAsync);
    } else {
        setValue(getDoubleClickReturnValue(), juce::sendNotificationAsync);
    }
}

void RotaryKnobSlider::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (isScrollWheelEnabled()) {
        double delta = (wheel.deltaY != 0.0f ? wheel.deltaY : wheel.deltaX);
        float sensitivity = e.mods.isShiftDown() ? 0.015f : 0.05f;
        double range = getMaximum() - getMinimum();
        double targetVal = std::clamp(getValue() + delta * sensitivity * range, getMinimum(), getMaximum());
        setValue(targetVal, juce::sendNotificationAsync);
    }
}

// --- SLIDER CALLOUT COMPONENT ---

SliderCalloutComponent::SliderCalloutComponent(RotaryKnobSlider& ownerSlider,
                                               const juce::String& pId,
                                               std::function<TheKlangFarmerAudioProcessor::ParamModulationInfo(const juce::String&)> modGetter)
    : slider(ownerSlider), paramId(pId), getModInfo(std::move(modGetter))
{
    if (getModInfo && paramId.isNotEmpty()) {
        cachedInfo = getModInfo(paramId);
    }

    editor.setFont(juce::FontOptions(13.5f, juce::Font::bold));
    editor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff0e1117));
    editor.setColour(juce::TextEditor::textColourId, juce::Colour(0xffffffff));
    editor.setColour(juce::TextEditor::outlineColourId, slider.getAccentColour().withAlpha(0.8f));
    editor.setText(slider.getTextFromValue(slider.getValue()), false);
    addAndMakeVisible(editor);

    editor.onReturnKey = [this]() {
        juce::String text = editor.getText().trim();
        double newVal = slider.getValueFromText(text);
        slider.setValue(newVal, juce::sendNotificationAsync);
        if (auto* callout = findParentComponentOfClass<juce::CallOutBox>()) {
            callout->dismiss();
        }
    };
    editor.onEscapeKey = [this]() {
        if (auto* callout = findParentComponentOfClass<juce::CallOutBox>()) {
            callout->dismiss();
        }
    };

    int w = 210;
    int h = 62;
    if (cachedInfo.isModulated) {
        h += 16; // divider + header
        h += static_cast<int>(cachedInfo.sources.size()) * 18;
        h += 18; // range text
        h += 18; // live value text
        h += 8;  // padding
        lastLiveText = cachedInfo.liveValueText;
        startTimerHz(30);
    }
    setSize(w, h);
}

SliderCalloutComponent::~SliderCalloutComponent() {
    stopTimer();
}

void SliderCalloutComponent::visibilityChanged() {
    if (isVisible()) {
        editor.grabKeyboardFocus();
        editor.selectAll();
    }
}

void SliderCalloutComponent::timerCallback() {
    if (getModInfo && paramId.isNotEmpty()) {
        auto latest = getModInfo(paramId);
        if (latest.liveValueText != lastLiveText || latest.isModulated != cachedInfo.isModulated) {
            lastLiveText = latest.liveValueText;
            cachedInfo = latest;
            repaint();
        }
    }
}

void SliderCalloutComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff12141a));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(slider.getAccentColour().withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.2f);

    // Title
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.setColour(slider.getAccentColour());
    juce::Rectangle<float> titleBox(10.0f, 6.0f, bounds.getWidth() - 20.0f, 18.0f);
    g.drawText(slider.getLabel().toUpperCase(), titleBox, juce::Justification::centredLeft, true);

    if (cachedInfo.isModulated) {
        float curY = static_cast<float>(editor.getBottom()) + 8.0f;
        // Divider
        g.setColour(juce::Colour(0xff2a3242));
        g.drawHorizontalLine(static_cast<int>(curY), 10.0f, bounds.getWidth() - 10.0f);
        curY += 6.0f;

        // Header
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff8a99ad));
        g.drawText("ACTIVE MODULATION", 10.0f, curY, bounds.getWidth() - 20.0f, 14.0f, juce::Justification::centredLeft, true);
        curY += 16.0f;

        // Sources
        for (const auto& src : cachedInfo.sources) {
            g.setFont(juce::FontOptions(11.5f, juce::Font::plain));
            g.setColour(juce::Colour(0xffc5d2e3));
            juce::String labelStr = juce::String(juce::CharPointer_UTF8("\xe2\x80\xa2 ")) + src.name + ": ";
            g.drawText(labelStr, 12.0f, curY, bounds.getWidth() - 24.0f, 16.0f, juce::Justification::centredLeft, true);

            float nameW = juce::GlyphArrangement::getStringWidth(juce::Font(juce::FontOptions(11.5f, juce::Font::plain)), labelStr);
            g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
            g.setColour(slider.getAccentColour().brighter(0.2f));
            g.drawText(src.depthText, 12.0f + nameW, curY, bounds.getWidth() - 24.0f - nameW, 16.0f, juce::Justification::centredLeft, true);
            curY += 18.0f;
        }

        // Modulation range: Start -> Peak
        g.setFont(juce::FontOptions(11.5f, juce::Font::plain));
        g.setColour(juce::Colour(0xff8a99ad));
        g.drawText("Range:", 12.0f, curY, 48.0f, 16.0f, juce::Justification::centredLeft, true);
        g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
        g.setColour(juce::Colour(0xffedf2fa));
        g.drawText(cachedInfo.rangeText, 60.0f, curY, bounds.getWidth() - 72.0f, 16.0f, juce::Justification::centredLeft, true);
        curY += 18.0f;

        // Instantaneous live value
        g.setFont(juce::FontOptions(11.5f, juce::Font::plain));
        g.setColour(juce::Colour(0xff8a99ad));
        g.drawText("Live:", 12.0f, curY, 48.0f, 16.0f, juce::Justification::centredLeft, true);
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(slider.getAccentColour());
        g.drawText(cachedInfo.liveValueText, 60.0f, curY, bounds.getWidth() - 72.0f, 16.0f, juce::Justification::centredLeft, true);
    }
}

void SliderCalloutComponent::resized() {
    editor.setBounds(8, 26, getWidth() - 16, 24);
}

void RotaryKnobSlider::openHoveringEditor() {
    auto callout = std::make_unique<SliderCalloutComponent>(*this, paramId, getModInfoFunc);
    juce::CallOutBox::launchAsynchronously(std::move(callout), getScreenBounds(), nullptr);
}

juce::String RotaryKnobSlider::getTextFromValue(double val) {
    if (customFormatText) {
        return customFormatText(val);
    }
    return juce::Slider::getTextFromValue(val);
}

double RotaryKnobSlider::getValueFromText(const juce::String& text) {
    if (customParseText) {
        return customParseText(text);
    }
    return juce::Slider::getValueFromText(text);
}

void RotaryKnobSlider::drawDiagram(juce::Graphics& g, juce::Rectangle<float> area) {
    if (area.getWidth() <= 4.0f || area.getHeight() <= 4.0f) return;

    g.setColour(juce::Colour(0x55000000));
    g.fillRoundedRectangle(area.expanded(2.0f, 1.0f), 2.5f);
    g.setColour(juce::Colour(0x33ffffff));
    g.drawRoundedRectangle(area.expanded(2.0f, 1.0f), 2.5f, 0.8f);

    float val = static_cast<float>(getValue());

    if (diagramType == DiagramType::Waveform) {
        g.setColour(juce::Colour(0x30ffffff));
        g.drawHorizontalLine(static_cast<int>(area.getCentreY()), area.getX(), area.getRight());

        juce::Path p;
        constexpr int numPts = 32;
        for (int i = 0; i <= numPts; ++i) {
            float phase = static_cast<float>(i) / static_cast<float>(numPts);
            float waveY = TbdAudio::evaluateWaveform(phase, val);
            float px = area.getX() + phase * area.getWidth();
            float py = area.getCentreY() - waveY * (area.getHeight() * 0.44f);
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(juce::Colours::white);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (diagramType == DiagramType::EnvelopeSlope) {
        g.setColour(juce::Colour(0x30ffffff));
        g.drawHorizontalLine(static_cast<int>(area.getBottom() - 1.0f), area.getX(), area.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float y = TbdAudio::applyEnvelopeSlope(1.0f - t, val);
            float px = area.getX() + t * area.getWidth();
            float py = area.getBottom() - y * (area.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(juce::Colours::white);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (diagramType == DiagramType::VelocitySlope) {
        g.setColour(juce::Colour(0x30ffffff));
        g.drawHorizontalLine(static_cast<int>(area.getBottom() - 1.0f), area.getX(), area.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float y = TbdAudio::applyEnvelopeSlope(t, val);
            float px = area.getX() + t * area.getWidth();
            float py = area.getBottom() - y * (area.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(juce::Colours::white);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (diagramType == DiagramType::FilterSlope) {
        g.setColour(juce::Colour(0x30ffffff));
        g.drawHorizontalLine(static_cast<int>(area.getBottom() - 1.0f), area.getX(), area.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        float cutoffX = 0.55f;
        float exponent = 1.0f + val * 6.0f;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float mag = 1.0f;
            if (t > cutoffX) {
                float f = (t - cutoffX) / (1.0f - cutoffX);
                mag = std::clamp(1.0f - std::pow(f, 1.0f / exponent), 0.0f, 1.0f);
            }
            float px = area.getX() + t * area.getWidth();
            float py = area.getBottom() - mag * (area.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(juce::Colours::white);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

void RotaryKnobSlider::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    float cornerRadius = 4.0f;

    if (isLightTrough) {
        // 1. Recessed white/light satin trough
        juce::ColourGradient troughGrad(juce::Colour(0xfff8fafc), bounds.getX(), bounds.getY(),
                                       juce::Colour(0xffe2e7ef), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(troughGrad);
        g.fillRoundedRectangle(bounds, cornerRadius);

        // Sunken hardware shadow at top
        g.setColour(juce::Colour(0x28000000));
        g.drawHorizontalLine(static_cast<int>(bounds.getY() + 1.0f), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);
        g.setColour(juce::Colour(0x12000000));
        g.drawHorizontalLine(static_cast<int>(bounds.getY() + 2.0f), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);

        // Crisp dark-slate border
        bool isHot = isMouseOverOrDragging();
        g.setColour(isHot ? accentColour.withAlpha(0.9f) : juce::Colour(0xff7a8496));
        g.drawRoundedRectangle(bounds, cornerRadius, isHot ? 1.4f : 1.0f);
    } else {
        // 1. Recessed dark trough background
        g.setColour(juce::Colour(0xff090c12));
        g.fillRoundedRectangle(bounds, cornerRadius);

        // Trough border
        bool isHot = isMouseOverOrDragging();
        g.setColour(isHot ? accentColour.withAlpha(0.65f) : juce::Colour(0xff1e2535));
        g.drawRoundedRectangle(bounds, cornerRadius, 1.2f);
    }

    float pad = 1.5f;
    float innerX = bounds.getX() + pad;
    float innerY = bounds.getY() + pad;
    float innerW = bounds.getWidth() - 2.0f * pad;
    float innerH = bounds.getHeight() - 2.0f * pad;
    float innerRadius = 3.0f;

    // 2. Normalized fill position
    double rng = getMaximum() - getMinimum();
    float norm = (rng > 0.0) ? static_cast<float>((getValue() - getMinimum()) / rng) : 0.0f;
    norm = std::clamp(norm, 0.0f, 1.0f);

    juce::Rectangle<float> fillRect;
    bool hasFill = false;

    if (isBipolar) {
        float midX = innerX + innerW * 0.5f;

        if (norm > 0.501f) {
            float fillW = innerW * (norm - 0.5f);
            fillRect = juce::Rectangle<float>(midX, innerY, fillW, innerH);
            hasFill = true;
            juce::ColourGradient grad(accentColour.darker(0.45f), midX, innerY,
                                      accentColour.brighter(0.1f), midX + fillW, innerY, false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(fillRect, 2.0f);

            // Leading edge needle
            float needleX = midX + fillW;
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.drawVerticalLine(static_cast<int>(needleX - 1.0f), innerY, innerY + innerH);
            g.setColour(juce::Colours::white);
            g.drawVerticalLine(static_cast<int>(needleX), innerY, innerY + innerH);
        } else if (norm < 0.499f) {
            float fillW = innerW * (0.5f - norm);
            float startX = midX - fillW;
            fillRect = juce::Rectangle<float>(startX, innerY, fillW, innerH);
            hasFill = true;
            juce::ColourGradient grad(accentColour.brighter(0.1f), startX, innerY,
                                      accentColour.darker(0.45f), midX, innerY, false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(fillRect, 2.0f);

            // Leading edge needle
            float needleX = startX;
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.drawVerticalLine(static_cast<int>(needleX + 1.0f), innerY, innerY + innerH);
            g.setColour(juce::Colours::white);
            g.drawVerticalLine(static_cast<int>(needleX), innerY, innerY + innerH);
        }

        // Center dividing needle
        g.setColour(isLightTrough ? juce::Colour(0xff222732) : accentColour.withAlpha(0.85f));
        g.drawVerticalLine(static_cast<int>(midX), innerY, innerY + innerH);
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.drawVerticalLine(static_cast<int>(midX), innerY + 1.0f, innerY + innerH - 1.0f);
    } else {
        float fillW = innerW * norm;
        if (fillW > 1.0f) {
            fillRect = juce::Rectangle<float>(innerX, innerY, fillW, innerH);
            hasFill = true;
            juce::ColourGradient grad(accentColour.withMultipliedSaturation(1.1f).darker(isLightTrough ? 0.35f : 0.6f),
                                      innerX, innerY,
                                      accentColour.brighter(0.12f),
                                      innerX + fillW, innerY, false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(fillRect, innerRadius);

            // Leading edge needle with glow
            float needleX = innerX + fillW;
            g.setColour(juce::Colours::white.withAlpha(0.45f));
            g.drawVerticalLine(static_cast<int>(needleX - 1.0f), innerY, innerY + innerH);
            g.setColour(juce::Colours::white);
            g.drawVerticalLine(static_cast<int>(needleX), innerY, innerY + innerH);
        }
    }

    // 2b. Modulation Range Bar & Indicator Needle (if modulated)
    if (modulation.isModulated) {
        float barH = 3.5f;
        float barY = innerY + innerH - barH - 1.5f;
        float minX = innerX + innerW * std::clamp(modulation.rangeMinNorm, 0.0f, 1.0f);
        float maxX = innerX + innerW * std::clamp(modulation.rangeMaxNorm, 0.0f, 1.0f);
        float spanW = std::max(2.5f, maxX - minX);

        juce::Rectangle<float> modBarRect(minX, barY, spanW, barH);
        g.setColour(accentColour.withAlpha(0.5f));
        g.fillRoundedRectangle(modBarRect, 1.5f);
        g.setColour(accentColour.brighter(0.3f).withAlpha(0.85f));
        g.drawRoundedRectangle(modBarRect, 1.5f, 0.8f);

        // Realtime indicator needle
        float currX = innerX + innerW * std::clamp(modulation.currentNorm, 0.0f, 1.0f);
        float indW = 4.0f;
        float indH = 8.0f;
        float indY = barY - 2.5f;
        juce::Rectangle<float> indRect(currX - indW * 0.5f, indY, indW, indH);

        g.setColour(accentColour.withAlpha(0.6f));
        g.fillRoundedRectangle(indRect.expanded(1.0f, 0.5f), 1.5f);
        g.setColour(juce::Colours::white);
        g.fillRoundedRectangle(indRect, 1.2f);
        g.setColour(juce::Colour(0xff090c12));
        g.drawVerticalLine(static_cast<int>(currX), indY + 1.0f, indY + indH - 1.0f);
    }

    // 3. Glass sheen reflection on top 44%
    float sheenH = innerH * 0.44f;
    juce::ColourGradient sheen(juce::Colour(isLightTrough ? 0x40ffffff : 0x35ffffff), innerX, innerY,
                               juce::Colour(0x04ffffff), innerX, innerY + sheenH, false);
    g.setGradientFill(sheen);
    g.fillRoundedRectangle(innerX, innerY, innerW, sheenH, 2.5f);

    // 4. Fixed Right-Aligned Value Box
    // Statically anchored so text length variations never move or push other elements!
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    auto valStr = getTextFromValue(getValue());
    float valStrW = juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), valStr);
    float valueBoxW = std::max(78.0f, valStrW + 4.0f);
    constexpr float rightMargin = 8.0f;
    auto valueBox = juce::Rectangle<float>(bounds.getRight() - rightMargin - valueBoxW,
                                           bounds.getY(), valueBoxW, bounds.getHeight());

    // 5. Embedded Mini Diagram (if applicable)
    float labelRightLimit = valueBox.getX() - 6.0f;
    if (diagramType != DiagramType::None) {
        float diagW = 34.0f;
        float diagH = 16.0f;
        float diagX = bounds.getX() + 92.0f;
        float diagY = bounds.getCentreY() - diagH * 0.5f;
        drawDiagram(g, juce::Rectangle<float>(diagX, diagY, diagW, diagH));
        labelRightLimit = diagX - 6.0f;
    }

    // 6. Inside Left-Aligned Parameter Label
    constexpr float leftMargin = 10.0f;
    auto labelBox = juce::Rectangle<float>(bounds.getX() + leftMargin, bounds.getY(),
                                           std::max(10.0f, labelRightLimit - (bounds.getX() + leftMargin)),
                                           bounds.getHeight());
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));

    // 7. Render Text with High-Contrast / Inversion Support
    if (isLightTrough) {
        // Base pass: Crisp solid black text on the white trough
        g.setColour(juce::Colour(0xff101318));
        g.drawText(label.toUpperCase(), labelBox, juce::Justification::centredLeft, true);
        g.drawText(valStr, valueBox, juce::Justification::centredRight, false);

        // Clipped pass: Over the colored fill, invert text to crisp white with dark drop shadow
        if (hasFill && fillRect.getWidth() > 1.0f) {
            g.saveState();
            g.reduceClipRegion(fillRect.toNearestInt());

            // Shadow
            g.setColour(juce::Colour(0x90000000));
            g.drawText(label.toUpperCase(), labelBox.translated(1.0f, 1.0f), juce::Justification::centredLeft, true);
            g.drawText(valStr, valueBox.translated(1.0f, 1.0f), juce::Justification::centredRight, false);

            // Pure white text over fill
            g.setColour(juce::Colours::white);
            g.drawText(label.toUpperCase(), labelBox, juce::Justification::centredLeft, true);
            g.drawText(valStr, valueBox, juce::Justification::centredRight, false);

            g.restoreState();
        }
    } else {
        // Standard dark trough text rendering
        g.setColour(juce::Colour(0xd0000000));
        g.drawText(label.toUpperCase(), labelBox.translated(1.0f, 1.0f), juce::Justification::centredLeft, true);
        g.drawText(valStr, valueBox.translated(1.0f, 1.0f), juce::Justification::centredRight, false);

        g.setColour(juce::Colour(0xffedf2fa));
        g.drawText(label.toUpperCase(), labelBox, juce::Justification::centredLeft, true);
        g.drawText(valStr, valueBox, juce::Justification::centredRight, false);
    }
}

// --- LED SELECTOR COMPONENT ---

LedSelectorComponent::LedSelectorComponent(juce::Colour activeAccent)
    : accent(activeAccent)
{
    setOpaque(false);
}

void LedSelectorComponent::setItems(const juce::StringArray& newItems, int numColumns) {
    items = newItems;
    columns = std::max(1, numColumns);
    repaint();
}

void LedSelectorComponent::setSelectedIndex(int newIndex, juce::NotificationType notification) {
    if (newIndex >= 0 && newIndex < items.size() && newIndex != selectedIndex) {
        selectedIndex = newIndex;
        repaint();
        if (notification != juce::dontSendNotification && onChange) {
            onChange(selectedIndex);
        }
    }
}

juce::Rectangle<int> LedSelectorComponent::getItemBounds(int index) const {
    int n = items.size();
    if (n == 0 || index < 0 || index >= n) return {};
    int rows = (n + columns - 1) / columns;
    float colW = static_cast<float>(getWidth()) / static_cast<float>(columns);
    float rowH = static_cast<float>(getHeight()) / static_cast<float>(rows);

    int col = index % columns;
    int row = index / columns;
    return juce::Rectangle<int>(static_cast<int>(col * colW),
                                static_cast<int>(row * rowH),
                                static_cast<int>(colW),
                                static_cast<int>(rowH));
}

int LedSelectorComponent::getItemIndexAt(juce::Point<int> pos) const {
    int n = items.size();
    if (n == 0) return -1;
    int rows = (n + columns - 1) / columns;
    float colW = static_cast<float>(getWidth()) / static_cast<float>(columns);
    float rowH = static_cast<float>(getHeight()) / static_cast<float>(rows);

    int col = static_cast<int>(pos.x / colW);
    int row = static_cast<int>(pos.y / rowH);

    if (col < 0 || col >= columns || row < 0 || row >= rows) return -1;
    int idx = row * columns + col;
    if (idx >= 0 && idx < n) return idx;
    return -1;
}

void LedSelectorComponent::paint(juce::Graphics& g) {
    int n = items.size();
    if (n == 0) return;

    for (int i = 0; i < n; ++i) {
        auto r = getItemBounds(i).toFloat().reduced(2.0f, 1.0f);
        bool isSel = (i == selectedIndex);
        bool isHov = (i == hoveredIndex);

        // Tactile button background & border
        if (isSel) {
            juce::ColourGradient grad(accent.withAlpha(0.24f), r.getX(), r.getY(),
                                      accent.withAlpha(0.10f), r.getX(), r.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(r, 4.0f);
            g.setColour(accent.withAlpha(0.70f));
            g.drawRoundedRectangle(r, 4.0f, 1.2f);
        } else if (isHov) {
            g.setColour(juce::Colour(0xff222733));
            g.fillRoundedRectangle(r, 4.0f);
            g.setColour(juce::Colour(0xff3f495e));
            g.drawRoundedRectangle(r, 4.0f, 1.0f);
        } else {
            g.setColour(juce::Colour(0xff181b23));
            g.fillRoundedRectangle(r, 4.0f);
            g.setColour(juce::Colour(0xff2b3140));
            g.drawRoundedRectangle(r, 4.0f, 1.0f);
        }

        // LED dot indicator
        float ledSize = 6.0f;
        float ledX = r.getX() + 4.5f;
        float ledY = r.getCentreY() - ledSize * 0.5f;
        auto ledBounds = juce::Rectangle<float>(ledX, ledY, ledSize, ledSize);

        if (isSel) {
            g.setColour(accent.withAlpha(0.40f));
            g.fillEllipse(ledBounds.expanded(2.0f));
            g.setColour(accent);
            g.fillEllipse(ledBounds);
            g.setColour(juce::Colours::white);
            g.fillEllipse(ledBounds.reduced(1.2f));
        } else {
            g.setColour(juce::Colour(0xff20242e));
            g.fillEllipse(ledBounds);
            g.setColour(juce::Colour(0xff353d4c));
            g.drawEllipse(ledBounds, 0.8f);
        }

        auto textBounds = r.withTrimmedLeft(14.0f).withTrimmedRight(2.0f);
        g.setFont(juce::FontOptions(isSel ? 13.5f : 13.0f, juce::Font::bold));
        g.setColour(isSel ? juce::Colours::white : (isHov ? juce::Colour(0xffe6edf8) : juce::Colour(0xffb0bdd0)));
        g.drawFittedText(items[i], textBounds.toNearestInt(), juce::Justification::centredLeft, 1);
    }
}

void LedSelectorComponent::mouseMove(const juce::MouseEvent& e) {
    int idx = getItemIndexAt(e.getPosition());
    if (idx != hoveredIndex) {
        hoveredIndex = idx;
        repaint();
    }
}

void LedSelectorComponent::mouseExit(const juce::MouseEvent&) {
    if (hoveredIndex != -1) {
        hoveredIndex = -1;
        repaint();
    }
}

void LedSelectorComponent::mouseDown(const juce::MouseEvent& e) {
    int idx = getItemIndexAt(e.getPosition());
    if (idx >= 0 && idx < items.size()) {
        setSelectedIndex(idx, juce::sendNotification);
    }
}

// --- MODULE CARD COMPONENT ---

ModuleCardComponent::ModuleCardComponent(const juce::String& title, juce::Colour accentColour, PanelStyle style)
    : moduleTitle(title), accent(accentColour), panelStyle(style), oscilloscope(accentColour)
{
    addAndMakeVisible(oscilloscope);

    for (int i = 0; i < 4; ++i) {
        labels[i].setFont(juce::FontOptions(13.0f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffc5d0e0));
        labels[i].setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(labels[i]);
    }
}

void ModuleCardComponent::setSelectorAtBottom(bool atBottom) {
    selectorAtBottom = atBottom;
    resized();
}

void ModuleCardComponent::setKnobsLightTrough(bool lightTrough) {
    knobsLightTrough = lightTrough;
    for (int i = 0; i < 4; ++i) {
        if (knobs[i]) {
            knobs[i]->setLightTrough(knobsLightTrough);
        }
    }
    repaint();
}

void ModuleCardComponent::setPanelStyle(PanelStyle style) {
    panelStyle = style;
    for (int i = 0; i < 4; ++i) {
        if (knobs[i]) {
            knobs[i]->setLightTrough(knobsLightTrough);
        }
    }
    repaint();
}

void ModuleCardComponent::setLedSelector(LedSelectorComponent* selector) {
    ledSelector = selector;
    if (ledSelector) {
        ledSelector->setAccent(accent);
        addAndMakeVisible(ledSelector);
    }
}

void ModuleCardComponent::setSecondLedSelector(LedSelectorComponent* selector) {
    secondLedSelector = selector;
    if (secondLedSelector) {
        secondLedSelector->setAccent(accent);
        addAndMakeVisible(secondLedSelector);
    }
}

void ModuleCardComponent::setSelector(juce::ComboBox* box) {
    selectorBox = box;
    if (selectorBox) addAndMakeVisible(selectorBox);
}

void ModuleCardComponent::setKnob(int slotIndex, const juce::String& label, RotaryKnobSlider* slider) {
    if (slotIndex >= 0 && slotIndex < 4) {
        knobs[slotIndex] = slider;
        labels[slotIndex].setText(label, juce::dontSendNotification);
        labels[slotIndex].setVisible(false);
        if (slider) {
            slider->setLabel(label);
            slider->setAccentColour(accent);
            slider->setLightTrough(knobsLightTrough);
            addAndMakeVisible(slider);
        }
    }
}

void ModuleCardComponent::setKnobLabel(int slotIndex, const juce::String& label) {
    if (slotIndex >= 0 && slotIndex < 4) {
        labels[slotIndex].setText(label, juce::dontSendNotification);
        if (knobs[slotIndex]) {
            knobs[slotIndex]->setLabel(label);
        }
    }
}

void ModuleCardComponent::updateScope(const float* data, int numSamples) {
    oscilloscope.updateData(data, numSamples);
}

void ModuleCardComponent::setPlotMode(MiniOscilloscopeComponent::PlotMode mode) {
    oscilloscope.setPlotMode(mode);
}

void ModuleCardComponent::updateFilterParams(int type, int slope, float cutoffHz, float resonance) {
    oscilloscope.updateFilterParams(type, slope, cutoffHz, resonance);
}

void ModuleCardComponent::updateEqParams(float freqHz, float widthOct, float gainDb, float djFilter) {
    oscilloscope.updateEqParams(freqHz, widthOct, gainDb, djFilter);
}

void ModuleCardComponent::mouseDown(const juce::MouseEvent& /*e*/) {
    if (onCardClicked) onCardClicked();
}

void ModuleCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    if (panelStyle == PanelStyle::DoepferSilver) {
        // 1. Darker gunmetal silver background (~#606060: 0xff696e77 to 0xff545961)
        juce::ColourGradient aluGrad(juce::Colour(0xff696e77), bounds.getX(), bounds.getY(),
                                    juce::Colour(0xff545961), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(aluGrad);
        g.fillRoundedRectangle(bounds, 6.0f);

        // Subtle fine horizontal hairline brushed grain
        g.setColour(juce::Colour(0x0e000000));
        for (float y = bounds.getY() + 3.0f; y < bounds.getBottom() - 3.0f; y += 3.0f) {
            g.drawHorizontalLine(static_cast<int>(y), bounds.getX() + 6.0f, bounds.getRight() - 6.0f);
        }
        g.setColour(juce::Colour(0x0affffff));
        for (float y = bounds.getY() + 4.0f; y < bounds.getBottom() - 3.0f; y += 3.0f) {
            g.drawHorizontalLine(static_cast<int>(y), bounds.getX() + 6.0f, bounds.getRight() - 6.0f);
        }

        // Dark industrial bezel border
        g.setColour(juce::Colour(0xff181b22));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.2f);

        // 2. Corner countersunk rack screws
        auto drawScrew = [&](float cx, float cy) {
            float r = 4.5f;
            g.setColour(juce::Colour(0xff323742));
            g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
            juce::ColourGradient screwGrad(juce::Colour(0xff505866), cx, cy - r * 0.7f,
                                          juce::Colour(0xff222730), cx, cy + r * 0.7f, false);
            g.setGradientFill(screwGrad);
            g.fillEllipse(cx - r + 0.8f, cy - r + 0.8f, (r - 0.8f) * 2.0f, (r - 0.8f) * 2.0f);
            g.setColour(juce::Colour(0xff101217));
            g.drawLine(cx - 2.5f, cy - 1.5f, cx + 2.5f, cy + 1.5f, 1.2f);
        };
        drawScrew(bounds.getX() + 9.0f, bounds.getY() + 9.0f);
        drawScrew(bounds.getRight() - 9.0f, bounds.getY() + 9.0f);
        drawScrew(bounds.getX() + 9.0f, bounds.getBottom() - 9.0f);
        drawScrew(bounds.getRight() - 9.0f, bounds.getBottom() - 9.0f);

        // 3. Red accent strip at the top
        auto headerStrip = bounds.removeFromTop(3.0f);
        g.setColour(accent);
        g.fillRoundedRectangle(headerStrip, 2.0f);

        // 4. Solid black / dark screenprinted title with light relief shadow
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        g.setColour(juce::Colour(0x35ffffff));
        g.drawText(moduleTitle.toUpperCase(), 18, 5, getWidth() - 36, 18, juce::Justification::left, true);
        g.setColour(juce::Colour(0xff0e1116));
        g.drawText(moduleTitle.toUpperCase(), 18, 4, getWidth() - 36, 18, juce::Justification::left, true);
        // Thin screenprint divider under header
        g.setColour(juce::Colour(0xff14171e));
        g.drawHorizontalLine(24, bounds.getX() + 14.0f, bounds.getRight() - 14.0f);

        return;
    }

    auto compColour = accent.withRotatedHue(0.5f);
    auto panelBg = juce::Colour(0xff151821).interpolatedWith(compColour, 0.12f);
    auto panelBorder = juce::Colour(0xff222736).interpolatedWith(compColour, 0.15f);

    g.setColour(panelBg);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(panelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.setColour(accent);
    g.drawText(moduleTitle.toUpperCase(), 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void ModuleCardComponent::resized() {
    oscilloscope.setVisible(false); // Visualization is hosted in Slot 5
    for (int i = 0; i < 4; ++i) {
        labels[i].setVisible(false);
    }

    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(24); // Title header
    area.removeFromTop(6);

    if (selectorAtBottom) {
        // 4 slots: 3 knobs on rows 0..2, selector on row 3 (bottom)
        int totalRows = 4;
        int rowH = area.getHeight() / totalRows;
        int sliderH = 36;

        for (int i = 0; i < 3; ++i) {
            auto row = area.removeFromTop(rowH);
            if (knobs[i]) {
                knobs[i]->setVisible(true);
                knobs[i]->setBounds(row.withSizeKeepingCentre(row.getWidth(), std::min(row.getHeight() - 4, sliderH)));
            }
        }
        if (knobs[3]) knobs[3]->setVisible(false);

        auto row = area.removeFromTop(rowH);
        if (ledSelector != nullptr) {
            ledSelector->setVisible(true);
            ledSelector->setBounds(row.withSizeKeepingCentre(row.getWidth(), std::min(row.getHeight() - 4, 30)));
        } else if (selectorBox != nullptr) {
            selectorBox->setVisible(true);
            selectorBox->setBounds(row.withSizeKeepingCentre(row.getWidth(), std::min(row.getHeight() - 4, 28)));
        }
        return;
    }

    int count = 4;
    if (ledSelector != nullptr && secondLedSelector != nullptr) {
        int selH1 = 28;
        int selH2 = 28;
        ledSelector->setBounds(area.removeFromTop(selH1));
        area.removeFromTop(5);
        secondLedSelector->setBounds(area.removeFromTop(selH2));
        area.removeFromTop(8);
        count = 2;
        if (knobs[2]) knobs[2]->setVisible(false);
        if (knobs[3]) knobs[3]->setVisible(false);
    } else if (ledSelector != nullptr) {
        int selH = 30;
        ledSelector->setBounds(area.removeFromTop(selH));
        area.removeFromTop(8);
        count = 3;
        if (knobs[3]) knobs[3]->setVisible(false);
    } else if (selectorBox != nullptr) {
        int selH = 28;
        selectorBox->setBounds(area.removeFromTop(selH));
        area.removeFromTop(8);
        count = 3;
        if (knobs[3]) knobs[3]->setVisible(false);
    }

    int rowH = area.getHeight() / count;
    int sliderH = (count == 4) ? 36 : (count == 3 ? 38 : 40);

    for (int i = 0; i < count; ++i) {
        auto row = area.removeFromTop(rowH);
        if (knobs[i]) {
            knobs[i]->setVisible(true);
            knobs[i]->setBounds(row.withSizeKeepingCentre(row.getWidth(), std::min(row.getHeight() - 4, sliderH)));
        }
    }
}

// --- FX SLOT CARD COMPONENT ---

FXSlotCardComponent::FXSlotCardComponent(int slot, bool isPostRack)
    : slotIndex(slot), isPost(isPostRack),
      selector1(juce::Colour(0xff00d2ff)),
      selector2(juce::Colour(0xff00d2ff))
{
    addChildComponent(&selector1);
    addChildComponent(&selector2);

    for (int i = 0; i < 4; ++i) {
        labels[i].setFont(juce::FontOptions(13.0f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffc5d0e0));
        labels[i].setJustificationType(juce::Justification::centredLeft);
        labels[i].setVisible(false);

        knobs[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knobs[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knobs[i].setScrollWheelEnabled(true);
        knobs[i].setRange(0.0, 1.0, 0.0005);
        addAndMakeVisible(knobs[i]);
    }
}

void FXSlotCardComponent::mouseDown(const juce::MouseEvent& /*e*/) {
    if (onCardClicked) onCardClicked();
}

void FXSlotCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    auto compColour = accent.withRotatedHue(0.5f);
    auto panelBg = juce::Colour(0xff151821).interpolatedWith(compColour, 0.12f);
    auto panelBorder = juce::Colour(0xff222736).interpolatedWith(compColour, 0.15f);

    g.setColour(panelBg);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(panelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.setColour(accent);
    juce::String headerText = (isPost ? "POST " : "PRE ") + juce::String(slotIndex + 1) + ": " + title.toUpperCase();
    g.drawText(headerText, 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void FXSlotCardComponent::resized() {
    for (int i = 0; i < 4; ++i) {
        labels[i].setVisible(false);
    }

    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(24); // Title header
    area.removeFromTop(6);

    std::vector<int> visibleKnobs;
    for (int i = 0; i < 4; ++i) {
        if (knobs[i].isVisible()) visibleKnobs.push_back(i);
    }

    if (selector1.isVisible() && selector2.isVisible()) {
        int selH1 = 28;
        int selH2 = 28;
        selector1.setBounds(area.removeFromTop(selH1));
        area.removeFromTop(5);
        selector2.setBounds(area.removeFromTop(selH2));
        area.removeFromTop(8);
    } else if (selector1.isVisible()) {
        int selH = 30;
        selector1.setBounds(area.removeFromTop(selH));
        area.removeFromTop(8);
    }

    int count = static_cast<int>(visibleKnobs.size());
    if (count > 0) {
        int rowH = area.getHeight() / count;
        int sliderH = (count == 4) ? 36 : (count == 3 ? 38 : 40);

        for (int kIdx : visibleKnobs) {
            auto row = area.removeFromTop(rowH);
            knobs[kIdx].setBounds(row.withSizeKeepingCentre(row.getWidth(), std::min(row.getHeight() - 4, sliderH)));
        }
    }
}

void FXSlotCardComponent::configureForType(int fxType) {
    currentType = fxType;
    lastSel1 = -1;
    lastSel2 = -1;

    for (int i = 0; i < 4; ++i) {
        knobs[i].diagramType = RotaryKnobSlider::DiagramType::None;
        knobs[i].customFormatText = nullptr;
        knobs[i].customParseText = nullptr;
        knobs[i].setVisible(false);
        labels[i].setVisible(false);
    }
    selector1.setVisible(false);
    selector2.setVisible(false);

    auto setupK = [this](int kIdx, const juce::String& name, bool bipolar,
                         std::function<juce::String(double)> fmt,
                         std::function<double(const juce::String&)> prs) {
        knobs[kIdx].setVisible(true);
        labels[kIdx].setText(name, juce::dontSendNotification);
        labels[kIdx].setVisible(false);
        knobs[kIdx].setLabel(name);
        knobs[kIdx].setAccentColour(accent);
        knobs[kIdx].setBipolar(bipolar);
        knobs[kIdx].setColour(juce::Slider::rotarySliderFillColourId, accent);
        knobs[kIdx].setColour(juce::Slider::trackColourId, accent);
        knobs[kIdx].customFormatText = fmt;
        knobs[kIdx].customParseText = prs;
        knobs[kIdx].repaint();
        knobs[kIdx].updateText();
    };

    switch (fxType) {
        case 1: { // Bell EQ
            title = "Bell EQ";
            accent = juce::Colour(0xff29b6f6);
            setupK(0, "Freq", false, formatEqFreqHz, parseEqFreqHz);
            setupK(1, "Width", false, formatEqWidthOct, parseEqWidthOct);
            setupK(2, "Gain", true, formatBipolarDb, parseBipolarDb);
            setupK(3, "DJ Filter", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 2: { // Chorus
            title = "Chorus";
            accent = juce::Colour(0xff38bdf8);
            setupK(0, "Rate", false, formatChorusRate, parseChorusRate);
            setupK(1, "Depth", false, formatPercent, parsePercent);
            setupK(2, "Feedback", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Mix", false, formatPercent, parsePercent);
            break;
        }
        case 3: { // Comb Filter
            title = "Comb Filter";
            accent = juce::Colour(0xff26a69a);
            setupK(0, "Dampening", false, formatFreqHz, parseFreqHz);
            setupK(1, "Cutoff", false, formatFreqHz, parseFreqHz);
            setupK(2, "Resonance", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Mix", true, formatWetDry, parseWetDry);
            knobs[3].setDoubleClickReturnValue(true, 0.5); // 0%:100% (Dry)
            knobs[3].getDefaultValue = []() { return 0.5; };
            break;
        }
        case 4: { // Phase Smear
            title = "Phase Smear";
            accent = juce::Colour(0xffec407a);
            selector1.setVisible(true);
            selector1.setAccent(accent);
            selector1.setItems({ "2nd", "4th" }, 2);
            selector1.onChange = [this](int idx) {
                knobs[0].setValue(idx == 0 ? 0.0 : 1.0, juce::sendNotification);
            };
            setupK(1, "Amount", false, formatStages, parseStages);
            setupK(2, "Cutoff", false, formatFreqHz, parseFreqHz);
            setupK(3, "Resonance", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 5: { // Drive
            title = "Drive";
            accent = juce::Colour(0xffff4081);
            selector1.setVisible(true);
            selector1.setAccent(accent);
            selector1.setItems({ "Off", "On" }, 2);
            selector1.onChange = [this](int idx) {
                knobs[3].setValue(idx == 0 ? 0.0 : 1.0, juce::sendNotification);
            };
            setupK(0, "Drive", false, formatDb, parseDb);
            knobs[0].setDoubleClickReturnValue(true, 0.2); // 0 dB
            knobs[0].getDefaultValue = []() { return 0.2; }; // 0 dB
            setupK(1, "Bias", true, formatBipolarPercent, parseBipolarPercent);
            setupK(2, "Filter", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 6: { // Filter
            title = "FX Filter";
            accent = juce::Colour(0xff7c4dff);
            selector1.setVisible(true);
            selector1.setAccent(accent);
            selector1.setItems({ "LPF", "BPF", "HPF", "BRF" }, 4);
            selector1.onChange = [this](int idx) {
                knobs[0].setValue(idx / 3.0, juce::sendNotification);
            };
            selector2.setVisible(true);
            selector2.setAccent(accent);
            selector2.setItems({ "6", "12", "18", "24", "36" }, 5);
            selector2.onChange = [this](int idx) {
                knobs[1].setValue(idx * 0.25, juce::sendNotification);
            };
            setupK(2, "Cutoff", false, formatFreqHz, parseFreqHz);
            setupK(3, "Resonance", false, formatPercent, parsePercent);
            break;
        }
        case 7: { // Flanger
            title = "Flanger";
            accent = juce::Colour(0xffec4899);
            setupK(0, "Rate", false, formatFlangerRate, parseFlangerRate);
            setupK(1, "Depth", false, formatPercent, parsePercent);
            setupK(2, "Feedback", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Mix", false, formatPercent, parsePercent);
            break;
        }
        case 8: { // Frequency Shifter
            title = "Freq Shifter";
            accent = juce::Colour(0xff00e676);
            auto fmtShift = [this](double val) {
                float maxRange = TbdAudio::normToRangeHz(static_cast<float>(knobs[1].getValue()));
                float hz = static_cast<float>((val - 0.5) * 2.0 * maxRange);
                return (hz > 0 ? "+" : "") + juce::String(hz, 1) + " Hz";
            };
            auto prsShift = [this](const juce::String& text) {
                float maxRange = TbdAudio::normToRangeHz(static_cast<float>(knobs[1].getValue()));
                double hz = parseNumberSafe(text, 0.0);
                return std::clamp((hz / (2.0 * maxRange)) + 0.5, 0.0, 1.0);
            };
            setupK(0, "Shift", true, fmtShift, prsShift);
            setupK(1, "Range", false, formatRangeHz, parseRangeHz);
            setupK(2, "Blend", true, formatWetDry, parseWetDry);
            knobs[2].setDoubleClickReturnValue(true, 0.5); // 0%:100% (Dry)
            knobs[2].getDefaultValue = []() { return 0.5; };
            setupK(3, "Width", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 9: { // Grit FX
            title = "Grit FX";
            accent = juce::Colour(0xffff9100);
            setupK(0, "Bit Rate", false, formatBits, parseBits);
            setupK(1, "Sample Rate", false, formatFreqHz, parseFreqHz);
            setupK(2, "Low", true, formatBipolarDb, parseBipolarDb);
            setupK(3, "High", true, formatBipolarDb, parseBipolarDb);
            break;
        }
        case 10: { // Phaser
            title = "Phaser";
            accent = juce::Colour(0xffa855f7);
            setupK(0, "Rate", false, formatPhaserRate, parsePhaserRate);
            setupK(1, "Depth", false, formatPercent, parsePercent);
            setupK(2, "Feedback", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Mix", false, formatPercent, parsePercent);
            break;
        }
        case 11: { // RingMod
            title = "RingMod FX";
            accent = juce::Colour(0xffff7043);
            setupK(0, "Waveform", false, formatPercent, parsePercent);
            knobs[0].diagramType = RotaryKnobSlider::DiagramType::Waveform;
            setupK(1, "Rate", false, formatFreqHz, parseFreqHz);
            setupK(2, "Amount", false, formatPercent, parsePercent);
            setupK(3, "Width", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 12: { // Tempo Delay
            title = "Tempo Delay";
            accent = juce::Colour(0xff10b981);
            setupK(0, "Division", false, formatDelayDiv, parseDelayDiv);
            setupK(1, "Feedback", false, formatPercent, parsePercent);
            setupK(2, "Tone", false, formatDelayTone, parseDelayTone);
            setupK(3, "Mix", false, formatPercent, parsePercent);
            break;
        }
        case 13: { // Wave Folder
            title = "Wave Folder";
            accent = juce::Colour(0xffff5252);
            selector1.setVisible(true);
            selector1.setAccent(accent);
            selector1.setItems({ "Off", "On" }, 2);
            selector1.onChange = [this](int idx) {
                knobs[0].setValue(idx == 0 ? 0.0 : 1.0, juce::sendNotification);
            };
            setupK(1, "Fold", false, formatWavefolds, parseWavefolds);
            setupK(2, "Bias", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Filter", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        default: { // None
            title = "Empty Slot";
            accent = juce::Colour(0xff75849b);
            break;
        }
    }

    resized();
    repaint();
}

void FXSlotCardComponent::updateDynamicControls() {
    if (currentType == 5) { // Drive: knob 3 is Limiter Off/On
        int sel = (knobs[3].getValue() >= 0.5) ? 1 : 0;
        if (sel != lastSel1) {
            selector1.setSelectedIndex(sel, juce::dontSendNotification);
            lastSel1 = sel;
        }
    } else if (currentType == 6) { // Filter: knob 0 is Type, knob 1 is Slope
        int sel1 = std::clamp(static_cast<int>(std::round(knobs[0].getValue() * 3.0)), 0, 3);
        if (sel1 != lastSel1) {
            selector1.setSelectedIndex(sel1, juce::dontSendNotification);
            lastSel1 = sel1;
        }
        int sel2 = std::clamp(static_cast<int>(std::round(knobs[1].getValue() * 4.0)), 0, 4);
        if (sel2 != lastSel2) {
            selector2.setSelectedIndex(sel2, juce::dontSendNotification);
            lastSel2 = sel2;
        }
    } else if (currentType == 4) { // Phase Smear: knob 0 is Order (0: 2nd, 1: 4th)
        int sel = (knobs[0].getValue() >= 0.5) ? 1 : 0;
        if (sel != lastSel1) {
            selector1.setSelectedIndex(sel, juce::dontSendNotification);
            lastSel1 = sel;
        }
    } else if (currentType == 13) { // WaveFolder: knob 0 is Off/On
        int sel = (knobs[0].getValue() >= 0.5) ? 1 : 0;
        if (sel != lastSel1) {
            selector1.setSelectedIndex(sel, juce::dontSendNotification);
            lastSel1 = sel;
        }
    }
}

// --- BLANK PLATE COMPONENT ---

void BlankPlateComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient grad(juce::Colour(0xff161920), bounds.getX(), bounds.getY(),
                              juce::Colour(0xff0d0f14), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto drawScrew = [&](float cx, float cy) {
        g.setColour(juce::Colour(0xff1f2430));
        g.fillEllipse(cx - 5.0f, cy - 5.0f, 10.0f, 10.0f);
        g.setColour(juce::Colour(0xff384254));
        g.drawEllipse(cx - 5.0f, cy - 5.0f, 10.0f, 10.0f, 1.0f);
        g.setColour(juce::Colour(0xff101318));
        g.drawLine(cx - 3.0f, cy, cx + 3.0f, cy, 1.2f);
    };

    drawScrew(bounds.getX() + 12.0f, bounds.getY() + 12.0f);
    drawScrew(bounds.getRight() - 12.0f, bounds.getY() + 12.0f);
    drawScrew(bounds.getX() + 12.0f, bounds.getBottom() - 12.0f);
    drawScrew(bounds.getRight() - 12.0f, bounds.getBottom() - 12.0f);

    g.setColour(juce::Colour(0xff1e222d));
    g.drawHorizontalLine(static_cast<int>(bounds.getCentreY()), bounds.getX() + 24.0f, bounds.getRight() - 24.0f);
}

// --- NAVIGATION CARD COMPONENT ---

NavigationCardComponent::NavigationCardComponent() {
    for (int i = 0; i < pageNames.size(); ++i) {
        auto btn = std::make_unique<juce::TextButton>(pageNames[i]);
        btn->setClickingTogglesState(false);
        int pageIdx = i;
        btn->onClick = [this, pageIdx]() {
            setSelectedPage(pageIdx);
            if (onPageSelected) onPageSelected(pageIdx);
        };
        addAndMakeVisible(btn.get());
        buttons.push_back(std::move(btn));
    }
    setSelectedPage(0);
}

void NavigationCardComponent::setSelectedPage(int pageIndex) {
    selectedPage = std::clamp(pageIndex, 0, (int)pageNames.size() - 1);
    for (int i = 0; i < (int)buttons.size(); ++i) {
        if (i == selectedPage) {
            buttons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d2ff));
            buttons[i]->setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
        } else {
            buttons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181c26));
            buttons[i]->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffb8c4d8));
        }
    }
    repaint();
}

void NavigationCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff151821));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(juce::Colour(0xff00d2ff));
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff00d2ff));
    g.drawText("PAGES", 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void NavigationCardComponent::resized() {
    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(22);
    area.removeFromTop(6);

    int n = static_cast<int>(buttons.size());
    if (n == 0) return;
    int gap = 6;
    int btnH = (area.getHeight() - (n - 1) * gap) / n;

    for (int i = 0; i < n; ++i) {
        buttons[i]->setBounds(area.removeFromTop(btnH));
        area.removeFromTop(gap);
    }
}

// --- FX PICKER CARD COMPONENT ---

FXPickerCardComponent::FXPickerCardComponent(const juce::String& titleText, juce::Colour accentCol)
    : title(titleText), accent(accentCol)
{
    const char* slotNames[4] = { "FX SLOT 1", "FX SLOT 2", "FX SLOT 3", "FX SLOT 4" };
    for (int i = 0; i < 4; ++i) {
        labels[i].setText(slotNames[i], juce::dontSendNotification);
        labels[i].setFont(juce::FontOptions(13.5f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffc5d0e0));
        addAndMakeVisible(labels[i]);

        boxes[i].setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff14161d));
        boxes[i].setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2c3240));
        boxes[i].setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8edf5));
        boxes[i].setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff8b95a8));
        addAndMakeVisible(boxes[i]);
    }
}

void FXPickerCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff151821));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.setColour(accent);
    g.drawText(title.toUpperCase(), 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void FXPickerCardComponent::resized() {
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(22);
    area.removeFromTop(8);

    int rowH = area.getHeight() / 4;
    for (int i = 0; i < 4; ++i) {
        auto row = area.removeFromTop(rowH).reduced(0, 4);
        labels[i].setBounds(row.removeFromTop(20));
        row.removeFromTop(4);
        boxes[i].setBounds(row.removeFromTop(28));
    }
}

// --- VISUALIZATION CARD COMPONENT ---

VisualizationCardComponent::VisualizationCardComponent(juce::Colour accentColour)
    : accent(accentColour), oscilloscope(accentColour)
{
    addAndMakeVisible(oscilloscope);
}

juce::Rectangle<int> VisualizationCardComponent::getLockBounds() const {
    return juce::Rectangle<int>(getWidth() - 34, 4, 26, 20);
}

void VisualizationCardComponent::setVisualizedBlock(int blockIndex, const juce::String& blockName) {
    if (isLocked) return;
    currentBlockIndex = blockIndex;
    currentBlockName = blockName.toUpperCase();
    repaint();
}

void VisualizationCardComponent::mouseDown(const juce::MouseEvent& e) {
    if (getLockBounds().contains(e.getPosition())) {
        isLocked = !isLocked;
        repaint();
    }
}

void VisualizationCardComponent::mouseMove(const juce::MouseEvent& e) {
    bool hovered = getLockBounds().contains(e.getPosition());
    if (hovered != isLockHovered) {
        isLockHovered = hovered;
        setMouseCursor(hovered ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void VisualizationCardComponent::mouseExit(const juce::MouseEvent& /*e*/) {
    if (isLockHovered) {
        isLockHovered = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void VisualizationCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto compColour = accent.withRotatedHue(0.5f);
    auto panelBg = juce::Colour(0xff151821).interpolatedWith(compColour, 0.12f);
    auto panelBorder = juce::Colour(0xff222736).interpolatedWith(compColour, 0.15f);

    g.setColour(panelBg);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(panelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    // Module name on top left
    g.setFont(juce::FontOptions(13.5f, juce::Font::bold));
    g.setColour(accent);
    juce::String headerText = "VISUALIZER: " + currentBlockName;
    g.drawText(headerText, 10, 4, getWidth() - 50, 20, juce::Justification::centredLeft, true);

    // Lock icon on top right: grey (0xff8892a4) when unlocked, bright yellow (0xffffd600) when locked
    auto lockR = getLockBounds().toFloat();
    juce::Colour lockColour = isLocked ? juce::Colour(0xffffd600) : juce::Colour(0xff8892a4);
    if (isLockHovered) lockColour = lockColour.brighter(0.25f);

    float cx = lockR.getCentreX();
    float cy = lockR.getCentreY();

    // Padlock body
    float bw = 12.0f;
    float bh = 9.0f;
    float bx = cx - bw * 0.5f;
    float by = cy - bh * 0.5f + 3.0f;
    juce::Rectangle<float> bodyRect(bx, by, bw, bh);
    g.setColour(lockColour);
    g.fillRoundedRectangle(bodyRect, 2.0f);

    // Padlock shackle
    juce::Path shackle;
    float sw = 8.0f;
    float sx = cx - sw * 0.5f;
    float archTop = by - 6.0f;

    if (isLocked) {
        shackle.startNewSubPath(sx + 1.0f, by);
        shackle.lineTo(sx + 1.0f, archTop + 3.0f);
        shackle.addCentredArc(cx, archTop + 3.0f, sw * 0.5f - 1.0f, sw * 0.5f - 1.0f, 0.0f, -juce::MathConstants<float>::pi, 0.0f, false);
        shackle.lineTo(sx + sw - 1.0f, by);
    } else {
        float openArchTop = archTop - 2.0f;
        shackle.startNewSubPath(sx + 1.0f, by);
        shackle.lineTo(sx + 1.0f, openArchTop + 3.0f);
        shackle.addCentredArc(cx, openArchTop + 3.0f, sw * 0.5f - 1.0f, sw * 0.5f - 1.0f, 0.0f, -juce::MathConstants<float>::pi, 0.0f, false);
    }
    g.strokePath(shackle, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Padlock keyhole
    g.setColour(juce::Colour(0xff151821));
    g.fillEllipse(cx - 1.2f, by + 2.5f, 2.4f, 2.4f);
    g.fillRect(cx - 0.7f, by + 4.0f, 1.4f, 2.5f);
}

void VisualizationCardComponent::resized() {
    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(24);
    oscilloscope.setBounds(area);
}

// --- QUICKSTART GUIDE MODAL COMPONENT ---

QuickstartGuideModalComponent::QuickstartGuideModalComponent() {
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff232938));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));
    closeButton.onClick = [this]() { setVisible(false); };
    addAndMakeVisible(closeButton);
    setWantsKeyboardFocus(true);
}

juce::Rectangle<int> QuickstartGuideModalComponent::getCardBounds() const {
    int cardW = std::min(920, getWidth() - 40);
    int cardH = std::min(640, getHeight() - 40);
    int cardX = (getWidth() - cardW) / 2;
    int cardY = (getHeight() - cardH) / 2;
    return juce::Rectangle<int>(cardX, cardY, cardW, cardH);
}

void QuickstartGuideModalComponent::resized() {
    auto card = getCardBounds();
    closeButton.setBounds(card.getRight() - 36, card.getY() + 10, 26, 26);
}

void QuickstartGuideModalComponent::mouseDown(const juce::MouseEvent& e) {
    if (!getCardBounds().contains(e.getPosition())) {
        setVisible(false);
    }
}

bool QuickstartGuideModalComponent::keyPressed(const juce::KeyPress& key) {
    if (key.isKeyCode(juce::KeyPress::escapeKey)) {
        setVisible(false);
        return true;
    }
    return false;
}

void QuickstartGuideModalComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xd00a0d14));

    auto card = getCardBounds().toFloat();

    g.setColour(juce::Colour(0xff12151c));
    g.fillRoundedRectangle(card, 8.0f);
    g.setColour(juce::Colour(0xff2a3245));
    g.drawRoundedRectangle(card.reduced(0.5f), 8.0f, 1.5f);

    auto topStrip = card.removeFromTop(4.0f);
    g.setColour(juce::Colour(0xff00d2ff));
    g.fillRoundedRectangle(topStrip, 2.0f);

    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("THE KLANG FARMER — QUICKSTART GUIDE", static_cast<int>(card.getX()) + 20, static_cast<int>(card.getY()) + 10, 500, 24, juce::Justification::left, true);

    g.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8892a4));
    g.drawText("Paged Modular Dual FM Drum Voice with 13 Multi-Instance Effects", static_cast<int>(card.getX()) + 20, static_cast<int>(card.getY()) + 34, 600, 18, juce::Justification::left, true);

    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(static_cast<int>(card.getY()) + 56, card.getX() + 16.0f, card.getRight() - 16.0f);

    auto contentArea = card;
    contentArea.removeFromTop(62.0f);
    contentArea.reduce(14.0f, 12.0f);

    float gap = 12.0f;
    float colW = (contentArea.getWidth() - gap) * 0.5f;
    float rowH = (contentArea.getHeight() - gap) * 0.5f;

    auto drawPanel = [&g](const juce::Rectangle<float>& r, const juce::String& title, juce::Colour accentCol, const juce::StringArray& bullets) {
        g.setColour(juce::Colour(0xff161a24));
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(juce::Colour(0xff242c3d));
        g.drawRoundedRectangle(r.reduced(0.5f), 6.0f, 1.0f);

        auto panelHeader = r;
        auto titleArea = panelHeader.removeFromTop(28.0f);

        g.setColour(accentCol);
        g.fillRoundedRectangle(titleArea.getX() + 10.0f, titleArea.getY() + 8.0f, 4.0f, 12.0f, 2.0f);

        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText(title, static_cast<int>(titleArea.getX()) + 20, static_cast<int>(titleArea.getY()) + 4, static_cast<int>(titleArea.getWidth()) - 30, 20, juce::Justification::left, true);

        float y = r.getY() + 32.0f;
        float x = r.getX() + 12.0f;
        float w = r.getWidth() - 24.0f;

        for (const auto& bullet : bullets) {
            g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
            g.setColour(juce::Colour(0xffc5d1e8));
            g.drawFittedText(bullet, static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), 32, juce::Justification::topLeft, 2);
            y += 35.0f;
        }
    };

    // Panel 1: Architecture & Signal Flow
    juce::Rectangle<float> p1(contentArea.getX(), contentArea.getY(), colW, rowH);
    juce::StringArray b1 = {
        "• DUAL FM VOICES: Two parallel voices each with Carrier (MIDI/Freq/Note), FM Modulator (Fixed/Follow/FM), dedicated Pitch Env, and Multimode Filter (LPF/BPF/HPF/BRF with 6-36dB slopes).",
        "• TRANSIENT NOISE: Analog-modeled White/Pink/Metallic noise source with dedicated Filter 3 and Filter Env for snappy clicks, snaps, and snare rattle.",
        "• 4-CHANNEL MIXER: Balance Carrier 1, Carrier 2, RingMod, and Noise Transients into the processing chain.",
        "• SIGNAL FLOW: Mixer -> Pre-Amp FX Rack (4 Slots) -> Pre-Limiter -> Amplifier + Amp Env -> Post-Amp FX Rack (4 Slots) -> Master Limiter -> Audio Output."
    };
    drawPanel(p1, "1. ARCHITECTURE & SIGNAL FLOW", juce::Colour(0xff00d2ff), b1);

    // Panel 2: Navigation & Smart Visualizer
    juce::Rectangle<float> p2(contentArea.getX() + colW + gap, contentArea.getY(), colW, rowH);
    juce::StringArray b2 = {
        "• 7-PAGE NAVIGATION (Slot 1): Instant 1-click access to Voice 1, Voice 2, Transients, Pre-Amp FX, Amplifier, Post-Amp FX, and Modulations.",
        "• AUTO-TRACKING VISUALIZER (Slot 5): Automatically switches to display real-time analysis for whichever module card or knob you click or edit.",
        "• BODE & OSCILLOSCOPE: Filters and EQ show interactive X-Y frequency response curves; Oscillators and FX display real-time triggered waveforms.",
        "• PADLOCK ICON (Top Right): Grey = auto-tracking active. Yellow = LOCKED! Lock visualizer to an FX (e.g. Wavefolder), then switch pages to sculpt sound while watching the locked waveform!"
    };
    drawPanel(p2, "2. NAVIGATION & AUTO-VISUALIZER", juce::Colour(0xffffd600), b2);

    // Panel 3: 13-Effects Engine & Multi-Instance
    juce::Rectangle<float> p3(contentArea.getX(), contentArea.getY() + rowH + gap, colW, rowH);
    juce::StringArray b3 = {
        "• 8 FX SLOTS: 4 Pre-Amp slots (pre-saturation) and 4 Post-Amp slots (post-saturation / spatial).",
        "• MULTI-INSTANCE: Assign ANY of the 13 effects to ANY slot. Stack up to 8 of the same effect in series if desired (e.g. multiple Wavefolders or cascading Filters)!",
        "• 13 DSP PROCESSORS: Bell EQ, Chorus, Comb Filter, Phase Smear, Drive, Filter, Flanger, Frequency Shifter, Grit FX, Phaser, RingMod, Tempo Delay, Wave Folder.",
        "• HARDWARE CONTROL: Standardized 4-knob tactile interface with illuminated LED button switches for quick, intuitive sound design."
    };
    drawPanel(p3, "3. MULTI-INSTANCE FX (13 EFFECTS)", juce::Colour(0xffff7043), b3);

    // Panel 4: Sound Design Recipes & Tips
    juce::Rectangle<float> p4(contentArea.getX() + colW + gap, contentArea.getY() + rowH + gap, colW, rowH);
    juce::StringArray b4 = {
        "• PUNCHY KICK: Voice 1 Carrier in MIDI mode (pitch ~36), Sine shape; Pitch Env fast decay (25ms), Depth +36st; Pre-Amp Wavefolder (Fold 2-4) + Drive (30%); Post-Limiter ON.",
        "• METALLIC SNARE: Voice 1 snappy body; Transients metallic noise with Filter 3 set to BPF (2kHz); Post-Amp Comb Filter or Chorus for stereo width.",
        "• MODULATIONS: 3 freely assignable Mod Envelopes on top row targetable to any parameter; Map Velocity, Key Tracking, and analog Slop on bottom row.",
        "• AUDITION HIT: Click the AUDITION HIT button in the top-right header at any time to audition the sound at full velocity."
    };
    drawPanel(p4, "4. SOUND DESIGN RECIPES & TIPS", juce::Colour(0xff00e5ff), b4);
}


// --- THE KLANG FARMER AUDIO PROCESSOR EDITOR CONSTRUCTOR ---

TheKlangFarmerAudioProcessorEditor::TheKlangFarmerAudioProcessorEditor(TheKlangFarmerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      carrier1TrackingSelector(juce::Colour(0xff00d2ff)),
      mod1TrackSelector(juce::Colour(0xffff7043)),
      mod1TypeSelector(juce::Colour(0xffff7043)),
      pitchEnv1TargetSelector(juce::Colour(0xffffab00)),
      filter1TypeSelector(juce::Colour(0xff7c4dff)),
      filter1SlopeSelector(juce::Colour(0xff7c4dff)),
      carrier2TrackingSelector(juce::Colour(0xff00d2ff)),
      mod2TrackSelector(juce::Colour(0xffff7043)),
      mod2TypeSelector(juce::Colour(0xffff7043)),
      pitchEnv2TargetSelector(juce::Colour(0xffffab00)),
      filter2TypeSelector(juce::Colour(0xff7c4dff)),
      filter2SlopeSelector(juce::Colour(0xff7c4dff)),
      filter3TypeSelector(juce::Colour(0xff7c4dff)),
      filter3SlopeSelector(juce::Colour(0xff7c4dff)),
      ampLimiterSelector(juce::Colour(0xffe53935)),
      preLimiterEnableSelector(juce::Colour(0xffe53935)),
      postLimiterEnableSelector(juce::Colour(0xffe53935)),
      vizCard(juce::Colour(0xff00d2ff))
{
    setLookAndFeel(&knobLookAndFeel);

    // Setup Header Quickstart Guide Button
    guideButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1f2430));
    guideButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff00d2ff));
    guideButton.onClick = [this]() {
        quickstartGuide.setVisible(true);
        quickstartGuide.toFront(true);
        quickstartGuide.grabKeyboardFocus();
    };
    addAndMakeVisible(guideButton);

    addChildComponent(quickstartGuide);

    // Setup Header Initialize Button
    initButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222736));
    initButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e8));
    initButton.onClick = [this]() {
        auto* alert = new juce::AlertWindow("Reset to Defaults",
                                           "Are you sure you want to reset all parameters to their default values?",
                                           juce::AlertWindow::QuestionIcon, this);
        alert->addButton("Reset", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alert->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        alert->setColour(juce::AlertWindow::backgroundColourId, juce::Colour(0xff151821));
        alert->setColour(juce::AlertWindow::textColourId, juce::Colour(0xffffffff));
        alert->setColour(juce::AlertWindow::outlineColourId, juce::Colour(0xff00d2ff));
        alert->enterModalState(true, juce::ModalCallbackFunction::create([this](int result) {
            if (result == 1) {
                resetToDefaults();
            }
        }), true);
    };
    addAndMakeVisible(initButton);

    // Setup Header Audition Trigger Button
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d2ff));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
    triggerButton.onClick = [this]() {
        audioProcessor.getEngine().trigger(1.0f);
    };
    addAndMakeVisible(triggerButton);

    // Permanent Slot 1 and Slot 8
    navCard.onPageSelected = [this](int pageIndex) {
        setPage(pageIndex);
    };
    addAndMakeVisible(navCard);

    addAndMakeVisible(vizCard);

    // Blank plates
    for (int i = 0; i < 6; ++i) {
        addChildComponent(&blankPlates[i]);
    }

    // FX Pickers
    preFXPickerCard = std::make_unique<FXPickerCardComponent>("PRE-AMP FX PICKER", juce::Colour(0xffff7043));
    addChildComponent(preFXPickerCard.get());

    postFXPickerCard = std::make_unique<FXPickerCardComponent>("POST-AMP FX PICKER", juce::Colour(0xff00e5ff));
    addChildComponent(postFXPickerCard.get());

    // --- CREATE MODULE CARDS & CONTROLS ---

    // 1. Carrier 1
    cardCarrier1 = std::make_unique<ModuleCardComponent>("Carrier 1", juce::Colour(0xff00d2ff));
    setupBox(carrier1TrackingBox);
    bindSelector(carrier1TrackingSelector, carrier1TrackingBox, "carrier1_tracking", { "MIDI", "Freq", "Note" }, 3);
    cardCarrier1->setLedSelector(&carrier1TrackingSelector);

    setupKnob(carrier1PitchSlider, juce::Colour(0xff00d2ff), true, 0.5);
    carrier1PitchSlider.customFormatText = [this](double val) {
        int mode = carrier1TrackingBox.getSelectedItemIndex();
        if (mode == 0) return formatSemi24(val);
        if (mode == 1) return formatCarrierFreqHz(val);
        return formatNoteDetail(val);
    };
    carrier1PitchSlider.customParseText = [this](const juce::String& text) {
        int mode = carrier1TrackingBox.getSelectedItemIndex();
        if (mode == 0) return parseSemi24(text);
        if (mode == 1) return parseCarrierFreqHz(text);
        return parseNoteDetail(text);
    };
    carrier1TrackingBox.onChange = [this]() {
        updateCarrier1Controls();
    };

    setupKnob(carrier1ShapeSlider, juce::Colour(0xff00d2ff), false, 0.0);
    carrier1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;

    setupKnob(carrier1DepthSlider, juce::Colour(0xff00d2ff), true, 0.5);
    carrier1DepthSlider.customFormatText = formatBipolarPercent;
    carrier1DepthSlider.customParseText  = parseBipolarPercent;

    cardCarrier1->setKnob(0, "Offset", &carrier1PitchSlider);
    cardCarrier1->setKnob(1, "Shape", &carrier1ShapeSlider);
    cardCarrier1->setKnob(2, "Mod Depth", &carrier1DepthSlider);
    addChildComponent(cardCarrier1.get());
    updateCarrier1Controls();

    // 2. Modulator 1
    cardMod1 = std::make_unique<ModuleCardComponent>("Modulator 1", juce::Colour(0xffff7043));
    setupBox(mod1TrackBox);
    bindSelector(mod1TrackSelector, mod1TrackBox, "mod1_track", { "Fixed", "Follow", "FM" }, 3);
    setupBox(mod1TypeBox);
    bindSelector(mod1TypeSelector, mod1TypeBox, "mod1_type", { "Osc", "Cyclic", "Noise" }, 3);
    cardMod1->setLedSelector(&mod1TrackSelector);
    cardMod1->setSecondLedSelector(&mod1TypeSelector);

    setupKnob(mod1ShapeSlider, juce::Colour(0xffff7043), false, 0.0);
    mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    mod1ShapeSlider.customFormatText = [this](double val) {
        if (mod1TypeBox.getSelectedItemIndex() == 1) return formatBipolarPercent(val);
        if (mod1TypeBox.getSelectedItemIndex() == 2) return formatFreqHz(val);
        return formatPercent(val);
    };
    mod1ShapeSlider.customParseText = [this](const juce::String& text) {
        if (mod1TypeBox.getSelectedItemIndex() == 1) return parseBipolarPercent(text);
        if (mod1TypeBox.getSelectedItemIndex() == 2) return parseFreqHz(text);
        return parsePercent(text);
    };

    setupKnob(mod1SpeedSlider, juce::Colour(0xffff7043), false, 0.50934);
    mod1SpeedSlider.customFormatText = [this](double val) {
        if (mod1TrackBox.getSelectedItemIndex() == 2) return formatRatio(val);
        if (mod1TrackBox.getSelectedItemIndex() == 1) return formatSemi(val);
        return formatFreqHz(val);
    };
    mod1SpeedSlider.customParseText = [this](const juce::String& text) {
        if (mod1TrackBox.getSelectedItemIndex() == 2) return parseRatio(text);
        if (mod1TrackBox.getSelectedItemIndex() == 1) return parseSemi(text);
        return parseFreqHz(text);
    };

    cardMod1->setKnob(0, "Shape", &mod1ShapeSlider);
    cardMod1->setKnob(1, "Speed", &mod1SpeedSlider);
    addChildComponent(cardMod1.get());

    // 3. Pitch Envelope 1
    cardPitchEnv1 = std::make_unique<ModuleCardComponent>("Pitch Env 1", juce::Colour(0xffffab00));
    setupBox(pitchEnv1TargetBox);
    bindSelector(pitchEnv1TargetSelector, pitchEnv1TargetBox, "pitchenv1_target", { "Car", "Mod", "Both", "Opp" }, 4);
    cardPitchEnv1->setSelectorAtBottom(true);
    cardPitchEnv1->setLedSelector(&pitchEnv1TargetSelector);

    setupKnob(pitchEnv1SlopeSlider, juce::Colour(0xffffab00), false, 0.5886);
    pitchEnv1SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    pitchEnv1SlopeSlider.customFormatText = formatSlope;
    pitchEnv1SlopeSlider.customParseText  = parseSlope;

    setupKnob(pitchEnv1DepthSlider, juce::Colour(0xffffab00), true, 0.5);
    pitchEnv1DepthSlider.customFormatText = formatOctaves;
    pitchEnv1DepthSlider.customParseText  = parseOctaves;

    setupKnob(pitchEnv1DecaySlider, juce::Colour(0xffffab00), false, 0.3806);
    pitchEnv1DecaySlider.customFormatText = formatTimeMs;
    pitchEnv1DecaySlider.customParseText  = parseTimeMs;

    cardPitchEnv1->setKnob(0, "Slope", &pitchEnv1SlopeSlider);
    cardPitchEnv1->setKnob(1, "Depth", &pitchEnv1DepthSlider);
    cardPitchEnv1->setKnob(2, "Decay", &pitchEnv1DecaySlider);
    addChildComponent(cardPitchEnv1.get());

    // 4. Filter 1
    cardFilter1 = std::make_unique<ModuleCardComponent>("Filter 1", juce::Colour(0xff7c4dff));
    setupBox(filter1TypeBox);
    bindSelector(filter1TypeSelector, filter1TypeBox, "filter1_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    setupBox(filter1SlopeBox);
    bindSelector(filter1SlopeSelector, filter1SlopeBox, "filter1_slope", { "6", "12", "18", "24", "36" }, 5);
    cardFilter1->setLedSelector(&filter1TypeSelector);
    cardFilter1->setSecondLedSelector(&filter1SlopeSelector);

    setupKnob(filter1CutoffSlider, juce::Colour(0xff7c4dff), false, 1.0);
    filter1CutoffSlider.customFormatText = formatFreqHz;
    filter1CutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filter1ResonanceSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filter1ResonanceSlider.customFormatText = formatPercent;
    filter1ResonanceSlider.customParseText  = parsePercent;

    cardFilter1->setKnob(0, "Cutoff", &filter1CutoffSlider);
    cardFilter1->setKnob(1, "Resonance", &filter1ResonanceSlider);
    addChildComponent(cardFilter1.get());

    // 5. Filter Envelope 1
    cardFilterEnv1 = std::make_unique<ModuleCardComponent>("Filter Env 1", juce::Colour(0xff7c4dff));
    setupKnob(filterEnv1SlopeSlider, juce::Colour(0xff7c4dff), false, 0.5886);
    filterEnv1SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnv1SlopeSlider.customFormatText = formatSlope;
    filterEnv1SlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnv1DepthSlider, juce::Colour(0xff7c4dff), true, 0.5);
    filterEnv1DepthSlider.customFormatText = formatFilterOctaves;
    filterEnv1DepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnv1DecaySlider, juce::Colour(0xff7c4dff), false, 0.3806);
    filterEnv1DecaySlider.customFormatText = formatTimeMs;
    filterEnv1DecaySlider.customParseText  = parseTimeMs;

    setupKnob(filterEnv1PostDriveSlider, juce::Colour(0xff7c4dff), false, 0.5);
    filterEnv1PostDriveSlider.customFormatText = formatDb;
    filterEnv1PostDriveSlider.customParseText  = parseDb;

    cardFilterEnv1->setKnob(0, "Slope", &filterEnv1SlopeSlider);
    cardFilterEnv1->setKnob(1, "Depth", &filterEnv1DepthSlider);
    cardFilterEnv1->setKnob(2, "Decay", &filterEnv1DecaySlider);
    cardFilterEnv1->setKnob(3, "Post-Drive", &filterEnv1PostDriveSlider);
    addChildComponent(cardFilterEnv1.get());

    // Voice 2 colors (swapped accent and complementary colors from Voice 1)
    const auto carrier2Colour = juce::Colour(0xff00d2ff).withRotatedHue(0.5f);
    const auto mod2Colour     = juce::Colour(0xffff7043).withRotatedHue(0.5f);
    const auto pitchEnv2Colour= juce::Colour(0xffffab00).withRotatedHue(0.5f);
    const auto filter2Colour  = juce::Colour(0xff7c4dff).withRotatedHue(0.5f);

    // 6. Carrier 2
    cardCarrier2 = std::make_unique<ModuleCardComponent>("Carrier 2", carrier2Colour);
    setupBox(carrier2TrackingBox);
    bindSelector(carrier2TrackingSelector, carrier2TrackingBox, "carrier2_tracking", { "MIDI", "Freq", "Note" }, 3);
    cardCarrier2->setLedSelector(&carrier2TrackingSelector);

    setupKnob(carrier2PitchSlider, carrier2Colour, true, 0.5);
    carrier2PitchSlider.customFormatText = [this](double val) {
        int mode = carrier2TrackingBox.getSelectedItemIndex();
        if (mode == 0) return formatSemi24(val);
        if (mode == 1) return formatCarrierFreqHz(val);
        return formatNoteDetail(val);
    };
    carrier2PitchSlider.customParseText = [this](const juce::String& text) {
        int mode = carrier2TrackingBox.getSelectedItemIndex();
        if (mode == 0) return parseSemi24(text);
        if (mode == 1) return parseCarrierFreqHz(text);
        return parseNoteDetail(text);
    };
    carrier2TrackingBox.onChange = [this]() {
        updateCarrier2Controls();
    };

    setupKnob(carrier2ShapeSlider, carrier2Colour, false, 0.0);
    carrier2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;

    setupKnob(carrier2DepthSlider, carrier2Colour, true, 0.5);
    carrier2DepthSlider.customFormatText = formatBipolarPercent;
    carrier2DepthSlider.customParseText  = parseBipolarPercent;

    cardCarrier2->setKnob(0, "Offset", &carrier2PitchSlider);
    cardCarrier2->setKnob(1, "Shape", &carrier2ShapeSlider);
    cardCarrier2->setKnob(2, "Mod Depth", &carrier2DepthSlider);
    addChildComponent(cardCarrier2.get());
    updateCarrier2Controls();

    // 7. Modulator 2
    cardMod2 = std::make_unique<ModuleCardComponent>("Modulator 2", mod2Colour);
    setupBox(mod2TrackBox);
    bindSelector(mod2TrackSelector, mod2TrackBox, "mod2_track", { "Fixed", "Follow", "FM" }, 3);
    setupBox(mod2TypeBox);
    bindSelector(mod2TypeSelector, mod2TypeBox, "mod2_type", { "Osc", "Cyclic", "Noise" }, 3);
    cardMod2->setLedSelector(&mod2TrackSelector);
    cardMod2->setSecondLedSelector(&mod2TypeSelector);

    setupKnob(mod2ShapeSlider, mod2Colour, false, 0.0);
    mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    mod2ShapeSlider.customFormatText = [this](double val) {
        if (mod2TypeBox.getSelectedItemIndex() == 1) return formatBipolarPercent(val);
        if (mod2TypeBox.getSelectedItemIndex() == 2) return formatFreqHz(val);
        return formatPercent(val);
    };
    mod2ShapeSlider.customParseText = [this](const juce::String& text) {
        if (mod2TypeBox.getSelectedItemIndex() == 1) return parseBipolarPercent(text);
        if (mod2TypeBox.getSelectedItemIndex() == 2) return parseFreqHz(text);
        return parsePercent(text);
    };

    setupKnob(mod2SpeedSlider, mod2Colour, false, 0.50934);
    mod2SpeedSlider.customFormatText = [this](double val) {
        if (mod2TrackBox.getSelectedItemIndex() == 2) return formatRatio(val);
        if (mod2TrackBox.getSelectedItemIndex() == 1) return formatSemi(val);
        return formatFreqHz(val);
    };
    mod2SpeedSlider.customParseText = [this](const juce::String& text) {
        if (mod2TrackBox.getSelectedItemIndex() == 2) return parseRatio(text);
        if (mod2TrackBox.getSelectedItemIndex() == 1) return parseSemi(text);
        return parseFreqHz(text);
    };

    cardMod2->setKnob(0, "Shape", &mod2ShapeSlider);
    cardMod2->setKnob(1, "Speed", &mod2SpeedSlider);
    addChildComponent(cardMod2.get());

    // 8. Pitch Envelope 2
    cardPitchEnv2 = std::make_unique<ModuleCardComponent>("Pitch Env 2", pitchEnv2Colour);
    setupBox(pitchEnv2TargetBox);
    bindSelector(pitchEnv2TargetSelector, pitchEnv2TargetBox, "pitchenv2_target", { "Car", "Mod", "Both", "Opp" }, 4);
    cardPitchEnv2->setSelectorAtBottom(true);
    cardPitchEnv2->setLedSelector(&pitchEnv2TargetSelector);

    setupKnob(pitchEnv2SlopeSlider, pitchEnv2Colour, false, 0.5886);
    pitchEnv2SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    pitchEnv2SlopeSlider.customFormatText = formatSlope;
    pitchEnv2SlopeSlider.customParseText  = parseSlope;

    setupKnob(pitchEnv2DepthSlider, pitchEnv2Colour, true, 0.5);
    pitchEnv2DepthSlider.customFormatText = formatOctaves;
    pitchEnv2DepthSlider.customParseText  = parseOctaves;

    setupKnob(pitchEnv2DecaySlider, pitchEnv2Colour, false, 0.3806);
    pitchEnv2DecaySlider.customFormatText = formatTimeMs;
    pitchEnv2DecaySlider.customParseText  = parseTimeMs;

    cardPitchEnv2->setKnob(0, "Slope", &pitchEnv2SlopeSlider);
    cardPitchEnv2->setKnob(1, "Depth", &pitchEnv2DepthSlider);
    cardPitchEnv2->setKnob(2, "Decay", &pitchEnv2DecaySlider);
    addChildComponent(cardPitchEnv2.get());

    // 9. Filter 2
    cardFilter2 = std::make_unique<ModuleCardComponent>("Filter 2", filter2Colour);
    setupBox(filter2TypeBox);
    bindSelector(filter2TypeSelector, filter2TypeBox, "filter2_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    setupBox(filter2SlopeBox);
    bindSelector(filter2SlopeSelector, filter2SlopeBox, "filter2_slope", { "6", "12", "18", "24", "36" }, 5);
    cardFilter2->setLedSelector(&filter2TypeSelector);
    cardFilter2->setSecondLedSelector(&filter2SlopeSelector);

    setupKnob(filter2CutoffSlider, filter2Colour, false, 1.0);
    filter2CutoffSlider.customFormatText = formatFreqHz;
    filter2CutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filter2ResonanceSlider, filter2Colour, false, 0.0);
    filter2ResonanceSlider.customFormatText = formatPercent;
    filter2ResonanceSlider.customParseText  = parsePercent;

    cardFilter2->setKnob(0, "Cutoff", &filter2CutoffSlider);
    cardFilter2->setKnob(1, "Resonance", &filter2ResonanceSlider);
    addChildComponent(cardFilter2.get());

    // 10. Filter Envelope 2
    cardFilterEnv2 = std::make_unique<ModuleCardComponent>("Filter Env 2", filter2Colour);
    setupKnob(filterEnv2SlopeSlider, filter2Colour, false, 0.5886);
    filterEnv2SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnv2SlopeSlider.customFormatText = formatSlope;
    filterEnv2SlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnv2DepthSlider, filter2Colour, true, 0.5);
    filterEnv2DepthSlider.customFormatText = formatFilterOctaves;
    filterEnv2DepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnv2DecaySlider, filter2Colour, false, 0.3806);
    filterEnv2DecaySlider.customFormatText = formatTimeMs;
    filterEnv2DecaySlider.customParseText  = parseTimeMs;

    setupKnob(filterEnv2PostDriveSlider, filter2Colour, false, 0.5);
    filterEnv2PostDriveSlider.customFormatText = formatDb;
    filterEnv2PostDriveSlider.customParseText  = parseDb;

    cardFilterEnv2->setKnob(0, "Slope", &filterEnv2SlopeSlider);
    cardFilterEnv2->setKnob(1, "Depth", &filterEnv2DepthSlider);
    cardFilterEnv2->setKnob(2, "Decay", &filterEnv2DecaySlider);
    cardFilterEnv2->setKnob(3, "Post-Drive", &filterEnv2PostDriveSlider);
    addChildComponent(cardFilterEnv2.get());

    // 11. Noise Transient
    cardNoise = std::make_unique<ModuleCardComponent>("Noise Transient", juce::Colour(0xff90a4ae));
    setupKnob(noiseShRateSlider, juce::Colour(0xff90a4ae), false, 1.0);
    noiseShRateSlider.customFormatText = formatFreqHz;
    noiseShRateSlider.customParseText  = parseFreqHz;

    setupKnob(noiseFilterSlider, juce::Colour(0xff90a4ae), true, 0.5);
    noiseFilterSlider.customFormatText = formatBipolarPercent;
    noiseFilterSlider.customParseText  = parseBipolarPercent;

    setupKnob(noiseDriveSlider, juce::Colour(0xff90a4ae), false, 0.5);
    noiseDriveSlider.customFormatText = formatDb;
    noiseDriveSlider.customParseText  = parseDb;

    setupKnob(noiseDecaySlider, juce::Colour(0xff90a4ae), false, 0.3078);
    noiseDecaySlider.customFormatText = formatNoiseTimeMs;
    noiseDecaySlider.customParseText  = parseNoiseTimeMs;

    cardNoise->setKnob(0, "S&H Rate", &noiseShRateSlider);
    cardNoise->setKnob(1, "Filter", &noiseFilterSlider);
    cardNoise->setKnob(2, "Drive", &noiseDriveSlider);
    cardNoise->setKnob(3, "Decay", &noiseDecaySlider);
    addChildComponent(cardNoise.get());

    // 12. Filter 3 (Transients Filter)
    cardFilter3 = std::make_unique<ModuleCardComponent>("Filter 3", juce::Colour(0xff7c4dff));
    setupBox(filter3TypeBox);
    bindSelector(filter3TypeSelector, filter3TypeBox, "filter3_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    setupBox(filter3SlopeBox);
    bindSelector(filter3SlopeSelector, filter3SlopeBox, "filter3_slope", { "6", "12", "18", "24", "36" }, 5);
    cardFilter3->setLedSelector(&filter3TypeSelector);
    cardFilter3->setSecondLedSelector(&filter3SlopeSelector);

    setupKnob(filter3CutoffSlider, juce::Colour(0xff7c4dff), false, 1.0);
    filter3CutoffSlider.customFormatText = formatFreqHz;
    filter3CutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filter3ResonanceSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filter3ResonanceSlider.customFormatText = formatPercent;
    filter3ResonanceSlider.customParseText  = parsePercent;

    cardFilter3->setKnob(0, "Cutoff", &filter3CutoffSlider);
    cardFilter3->setKnob(1, "Resonance", &filter3ResonanceSlider);
    addChildComponent(cardFilter3.get());

    // 13. Filter Envelope 3 (Transients Filter Env)
    cardFilterEnv3 = std::make_unique<ModuleCardComponent>("Filter Env 3", juce::Colour(0xff7c4dff));
    setupKnob(filterEnv3SlopeSlider, juce::Colour(0xff7c4dff), false, 0.5886);
    filterEnv3SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnv3SlopeSlider.customFormatText = formatSlope;
    filterEnv3SlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnv3DepthSlider, juce::Colour(0xff7c4dff), true, 0.5);
    filterEnv3DepthSlider.customFormatText = formatFilterOctaves;
    filterEnv3DepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnv3DecaySlider, juce::Colour(0xff7c4dff), false, 0.3078);
    filterEnv3DecaySlider.customFormatText = formatNoiseTimeMs;
    filterEnv3DecaySlider.customParseText  = parseNoiseTimeMs;

    setupKnob(filterEnv3PostDriveSlider, juce::Colour(0xff7c4dff), false, 0.5);
    filterEnv3PostDriveSlider.customFormatText = formatDb;
    filterEnv3PostDriveSlider.customParseText  = parseDb;

    cardFilterEnv3->setKnob(0, "Slope", &filterEnv3SlopeSlider);
    cardFilterEnv3->setKnob(1, "Depth", &filterEnv3DepthSlider);
    cardFilterEnv3->setKnob(2, "Decay", &filterEnv3DecaySlider);
    cardFilterEnv3->setKnob(3, "Post-Drive", &filterEnv3PostDriveSlider);
    addChildComponent(cardFilterEnv3.get());

    // 14. Mixer
    cardMixer = std::make_unique<ModuleCardComponent>("Mixer", juce::Colour(0xffe53935), ModuleCardComponent::PanelStyle::DoepferSilver);
    setupKnob(mixerCarrier1LevelSlider, juce::Colour(0xffe53935), false, 0.5);
    mixerCarrier1LevelSlider.setLightTrough(true);
    mixerCarrier1LevelSlider.customFormatText = formatMixerLevel;
    mixerCarrier1LevelSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerCarrier2LevelSlider, juce::Colour(0xffe53935), false, 0.0);
    mixerCarrier2LevelSlider.setDoubleClickReturnValue(true, 0.5);
    mixerCarrier2LevelSlider.setLightTrough(true);
    mixerCarrier2LevelSlider.customFormatText = formatMixerLevel;
    mixerCarrier2LevelSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerRingModSlider, juce::Colour(0xffe53935), false, 0.0);
    mixerRingModSlider.setDoubleClickReturnValue(true, 0.5);
    mixerRingModSlider.setLightTrough(true);
    mixerRingModSlider.customFormatText = formatMixerLevel;
    mixerRingModSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerNoiseLevelSlider, juce::Colour(0xffe53935), false, 0.0);
    mixerNoiseLevelSlider.setDoubleClickReturnValue(true, 0.5);
    mixerNoiseLevelSlider.setLightTrough(true);
    mixerNoiseLevelSlider.customFormatText = formatMixerLevel;
    mixerNoiseLevelSlider.customParseText  = parseMixerLevel;

    cardMixer->setKnob(0, "Carrier 1", &mixerCarrier1LevelSlider);
    cardMixer->setKnob(1, "Carrier 2", &mixerCarrier2LevelSlider);
    cardMixer->setKnob(2, "RingMod", &mixerRingModSlider);
    cardMixer->setKnob(3, "Noise", &mixerNoiseLevelSlider);
    addChildComponent(cardMixer.get());

    // Multi-Instance FX Slot Cards (4 Pre-Amp FX, 4 Post-Amp FX)
    for (int s = 0; s < 4; ++s) {
        preFXCards[s] = std::make_unique<FXSlotCardComponent>(s, false);
        addChildComponent(preFXCards[s].get());

        postFXCards[s] = std::make_unique<FXSlotCardComponent>(s, true);
        addChildComponent(postFXCards[s].get());
    }

    // 24. Amp
    cardAmp = std::make_unique<ModuleCardComponent>("Amplifier", juce::Colour(0xff00e5ff));
    setupBox(ampLimiterBox);
    bindSelector(ampLimiterSelector, ampLimiterBox, "amp_limiter", { "Off", "On" }, 2);
    ampLimiterSelector.setAccent(juce::Colour(0xffe53935));
    cardAmp->setLedSelector(&ampLimiterSelector);

    setupKnob(ampLevelSlider, juce::Colour(0xff00e5ff), false, 1.0);
    ampLevelSlider.customFormatText = formatPercent;
    ampLevelSlider.customParseText  = parsePercent;

    setupKnob(ampPanSlider, juce::Colour(0xff00e5ff), true, 0.5);
    ampPanSlider.customFormatText = [](double val) {
        int p = static_cast<int>(std::round((val - 0.5) * 200.0));
        if (p == 0) return juce::String("Center");
        return (p < 0 ? juce::String(-p) + "% L" : juce::String(p) + "% R");
    };
    ampPanSlider.customParseText = parseBipolarPercent;

    setupKnob(ampDriveSlider, juce::Colour(0xff00e5ff), false, 0.5);
    ampDriveSlider.customFormatText = formatDb;
    ampDriveSlider.customParseText  = parseDb;

    cardAmp->setKnob(0, "Level", &ampLevelSlider);
    cardAmp->setKnob(1, "Pan", &ampPanSlider);
    cardAmp->setKnob(2, "Drive", &ampDriveSlider);
    addChildComponent(cardAmp.get());

    // 25. Amp Envelope
    cardAmpEnv = std::make_unique<ModuleCardComponent>("Amp Envelope", juce::Colour(0xff00e5ff));
    setupKnob(ampEnvClapsSlider, juce::Colour(0xff00e5ff), false, 0.0);
    ampEnvClapsSlider.customFormatText = formatClaps;
    ampEnvClapsSlider.customParseText  = parseClaps;

    setupKnob(ampEnvClapSpeedSlider, juce::Colour(0xff00e5ff), false, 2.0 / 14.0);
    ampEnvClapSpeedSlider.customFormatText = formatClapSpeed;
    ampEnvClapSpeedSlider.customParseText  = parseClapSpeed;

    setupKnob(ampEnvSlopeSlider, juce::Colour(0xff00e5ff), false, 0.5886);
    ampEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    ampEnvSlopeSlider.customFormatText = formatSlope;
    ampEnvSlopeSlider.customParseText  = parseSlope;

    setupKnob(ampEnvDecaySlider, juce::Colour(0xff00e5ff), false, 0.3806);
    ampEnvDecaySlider.customFormatText = formatTimeMs;
    ampEnvDecaySlider.customParseText  = parseTimeMs;

    cardAmpEnv->setKnob(0, "Claps", &ampEnvClapsSlider);
    cardAmpEnv->setKnob(1, "Clap Speed", &ampEnvClapSpeedSlider);
    cardAmpEnv->setKnob(2, "Slope", &ampEnvSlopeSlider);
    cardAmpEnv->setKnob(3, "Decay", &ampEnvDecaySlider);
    addChildComponent(cardAmpEnv.get());

    // 26. Pre-Amp Limiter
    cardPreLimiter = std::make_unique<ModuleCardComponent>("Pre Limiter", juce::Colour(0xffe53935), ModuleCardComponent::PanelStyle::DoepferSilver);
    cardPreLimiter->setKnobsLightTrough(false);
    setupBox(preLimiterEnableBox);
    bindSelector(preLimiterEnableSelector, preLimiterEnableBox, "pre_limiter_enable", { "Off", "On" }, 2);
    preLimiterEnableSelector.setAccent(juce::Colour(0xffe53935));
    cardPreLimiter->setLedSelector(&preLimiterEnableSelector);

    setupKnob(preLimiterGainSlider, juce::Colour(0xffe53935), false, 12.0 / 36.0);
    preLimiterGainSlider.customFormatText = formatLimiterGain;
    preLimiterGainSlider.customParseText  = parseLimiterGain;

    setupKnob(preLimiterThreshSlider, juce::Colour(0xffe53935), false, 1.0);
    preLimiterThreshSlider.customFormatText = formatLimiterThresh;
    preLimiterThreshSlider.customParseText  = parseLimiterThresh;

    setupKnob(preLimiterReleaseSlider, juce::Colour(0xffe53935), false, 0.6296);
    preLimiterReleaseSlider.customFormatText = formatLimiterRelease;
    preLimiterReleaseSlider.customParseText  = parseLimiterRelease;

    cardPreLimiter->setKnob(0, "In Gain", &preLimiterGainSlider);
    cardPreLimiter->setKnob(1, "Thresh", &preLimiterThreshSlider);
    cardPreLimiter->setKnob(2, "Release", &preLimiterReleaseSlider);
    addChildComponent(cardPreLimiter.get());

    // 27. Post-Amp Limiter
    cardPostLimiter = std::make_unique<ModuleCardComponent>("Post Limiter", juce::Colour(0xffe53935), ModuleCardComponent::PanelStyle::DoepferSilver);
    cardPostLimiter->setKnobsLightTrough(false);
    setupBox(postLimiterEnableBox);
    bindSelector(postLimiterEnableSelector, postLimiterEnableBox, "post_limiter_enable", { "Off", "On" }, 2);
    postLimiterEnableSelector.setAccent(juce::Colour(0xffe53935));
    cardPostLimiter->setLedSelector(&postLimiterEnableSelector);

    setupKnob(postLimiterGainSlider, juce::Colour(0xffe53935), false, 12.0 / 36.0);
    postLimiterGainSlider.customFormatText = formatLimiterGain;
    postLimiterGainSlider.customParseText  = parseLimiterGain;

    setupKnob(postLimiterThreshSlider, juce::Colour(0xffe53935), false, 1.0);
    postLimiterThreshSlider.customFormatText = formatLimiterThresh;
    postLimiterThreshSlider.customParseText  = parseLimiterThresh;

    setupKnob(postLimiterReleaseSlider, juce::Colour(0xffe53935), false, 0.6296);
    postLimiterReleaseSlider.customFormatText = formatLimiterRelease;
    postLimiterReleaseSlider.customParseText  = parseLimiterRelease;

    cardPostLimiter->setKnob(0, "In Gain", &postLimiterGainSlider);
    cardPostLimiter->setKnob(1, "Thresh", &postLimiterThreshSlider);
    cardPostLimiter->setKnob(2, "Release", &postLimiterReleaseSlider);
    addChildComponent(cardPostLimiter.get());

    // 28. Velocity
    cardVelocity = std::make_unique<ModuleCardComponent>("Velocity", juce::Colour(0xff29b6f6));
    setupKnob(velSlopeSlider, juce::Colour(0xff29b6f6), false, 0.5886);
    velSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::VelocitySlope;
    velSlopeSlider.customFormatText = formatVelocitySlope;
    velSlopeSlider.customParseText  = parseVelocitySlope;

    setupKnob(velDepthSlider, juce::Colour(0xff29b6f6), true, 0.5);
    velDepthSlider.customFormatText = formatBipolarPercent;
    velDepthSlider.customParseText  = parseBipolarPercent;

    setupKnob(velDecaySlider, juce::Colour(0xff29b6f6), true, 0.5);
    velDecaySlider.customFormatText = formatBipolarPercent;
    velDecaySlider.customParseText  = parseBipolarPercent;

    setupKnob(velVolumeSlider, juce::Colour(0xff29b6f6), false, 0.0);
    velVolumeSlider.customFormatText = [](double val) {
        int v = static_cast<int>(std::round(val * 100.0));
        return "-" + juce::String(v) + "%";
    };
    velVolumeSlider.customParseText = parsePercent;

    cardVelocity->setKnob(0, "Slope", &velSlopeSlider);
    cardVelocity->setKnob(1, "Depth", &velDepthSlider);
    cardVelocity->setKnob(2, "Decay", &velDecaySlider);
    cardVelocity->setKnob(3, "Volume", &velVolumeSlider);
    addChildComponent(cardVelocity.get());

    // 29. Key Tracking
    cardKeyTrack = std::make_unique<ModuleCardComponent>("Key Track", juce::Colour(0xff26a69a));
    setupKnob(keySlopeSlider, juce::Colour(0xff26a69a), false, 0.5886);
    keySlopeSlider.diagramType = RotaryKnobSlider::DiagramType::VelocitySlope;
    keySlopeSlider.customFormatText = formatVelocitySlope;
    keySlopeSlider.customParseText  = parseVelocitySlope;

    setupKnob(keyDepthSlider, juce::Colour(0xff26a69a), true, 0.5);
    keyDepthSlider.customFormatText = formatBipolarPercent;
    keyDepthSlider.customParseText  = parseBipolarPercent;

    setupKnob(keyDecaySlider, juce::Colour(0xff26a69a), true, 0.5);
    keyDecaySlider.customFormatText = formatBipolarPercent;
    keyDecaySlider.customParseText  = parseBipolarPercent;

    setupKnob(keyVolumeSlider, juce::Colour(0xff26a69a), false, 0.0);
    keyVolumeSlider.customFormatText = [](double val) {
        int v = static_cast<int>(std::round(val * 100.0));
        return "-" + juce::String(v) + "%";
    };
    keyVolumeSlider.customParseText = parsePercent;

    cardKeyTrack->setKnob(0, "Slope", &keySlopeSlider);
    cardKeyTrack->setKnob(1, "Depth", &keyDepthSlider);
    cardKeyTrack->setKnob(2, "Decay", &keyDecaySlider);
    cardKeyTrack->setKnob(3, "Volume", &keyVolumeSlider);
    addChildComponent(cardKeyTrack.get());

    // 30. Slop
    cardSlop = std::make_unique<ModuleCardComponent>("Slop", juce::Colour(0xffab47bc));
    setupKnob(slopFreqSlider, juce::Colour(0xffab47bc), false, 0.0);
    slopFreqSlider.customFormatText = formatSlop;
    slopFreqSlider.customParseText  = parseSlop;

    setupKnob(slopDepthSlider, juce::Colour(0xffab47bc), false, 0.0);
    slopDepthSlider.customFormatText = formatSlop;
    slopDepthSlider.customParseText  = parseSlop;

    setupKnob(slopDecaySlider, juce::Colour(0xffab47bc), false, 0.0);
    slopDecaySlider.customFormatText = formatSlop;
    slopDecaySlider.customParseText  = parseSlop;

    setupKnob(slopPanSlider, juce::Colour(0xffab47bc), false, 0.0);
    slopPanSlider.customFormatText = formatSlop;
    slopPanSlider.customParseText  = parseSlop;

    cardSlop->setKnob(0, "Freq", &slopFreqSlider);
    cardSlop->setKnob(1, "Depth", &slopDepthSlider);
    cardSlop->setKnob(2, "Decay", &slopDecaySlider);
    cardSlop->setKnob(3, "Pan", &slopPanSlider);
    addChildComponent(cardSlop.get());

    // 31. Mod Envelopes 1..3
    const auto modChoices = TheKlangFarmerAudioProcessor::getModDestinationChoices();

    auto setupModEnvCard = [this, &modChoices](std::unique_ptr<ModuleCardComponent>& card,
                                               const juce::String& title,
                                               RotaryKnobSlider& slope,
                                               RotaryKnobSlider& depth,
                                               RotaryKnobSlider& decay,
                                               juce::ComboBox& targetBox) {
        card = std::make_unique<ModuleCardComponent>(title, juce::Colour(0xffffb300));
        targetBox.clear();
        targetBox.addItemList(modChoices, 1);
        targetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff161922));
        targetBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8edf5));
        targetBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2d3342));
        targetBox.setJustificationType(juce::Justification::centredLeft);
        card->setSelector(&targetBox);
        card->setSelectorAtBottom(true);

        setupKnob(slope, juce::Colour(0xffffb300), false, 0.5886);
        slope.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
        slope.customFormatText = formatSlope;
        slope.customParseText  = parseSlope;

        setupKnob(depth, juce::Colour(0xffffb300), true, 0.5);
        depth.customFormatText = formatBipolarPercent;
        depth.customParseText  = parseBipolarPercent;

        setupKnob(decay, juce::Colour(0xffffb300), false, 0.3806);
        decay.customFormatText = formatTimeMs;
        decay.customParseText  = parseTimeMs;

        card->setKnob(0, "Slope", &slope);
        card->setKnob(1, "Depth", &depth);
        card->setKnob(2, "Decay", &decay);
        addChildComponent(card.get());
    };

    setupModEnvCard(cardModEnv1, "Mod Env 1", modEnv1SlopeSlider, modEnv1DepthSlider, modEnv1DecaySlider, modEnv1TargetBox);
    setupModEnvCard(cardModEnv2, "Mod Env 2", modEnv2SlopeSlider, modEnv2DepthSlider, modEnv2DecaySlider, modEnv2TargetBox);
    setupModEnvCard(cardModEnv3, "Mod Env 3", modEnv3SlopeSlider, modEnv3DepthSlider, modEnv3DecaySlider, modEnv3TargetBox);
    updateModTargetBoxItems();

    // --- APVTS PARAMETER ATTACHMENTS ---

    // 1. Carrier 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier1_tracking", carrier1TrackingBox));
    bindSlider("carrier1_pitch", carrier1PitchSlider);
    bindSlider("carrier1_shape", carrier1ShapeSlider);
    bindSlider("carrier1_depth", carrier1DepthSlider);

    // 2. Modulator 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod1_track", mod1TrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod1_type", mod1TypeBox));
    bindSlider("mod1_shape", mod1ShapeSlider);
    bindSlider("mod1_speed", mod1SpeedSlider);

    // 3. Pitch Envelope 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pitchenv1_target", pitchEnv1TargetBox));
    bindSlider("pitchenv1_slope", pitchEnv1SlopeSlider);
    bindSlider("pitchenv1_depth", pitchEnv1DepthSlider);
    bindSlider("pitchenv1_decay", pitchEnv1DecaySlider);

    // 4. Filter 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter1_type", filter1TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter1_slope", filter1SlopeBox));
    bindSlider("filter1_cutoff", filter1CutoffSlider);
    bindSlider("filter1_resonance", filter1ResonanceSlider);

    // 5. Filter Envelope 1
    bindSlider("filterenv1_slope", filterEnv1SlopeSlider);
    bindSlider("filterenv1_depth", filterEnv1DepthSlider);
    bindSlider("filterenv1_decay", filterEnv1DecaySlider);
    bindSlider("filterenv1_postdrive", filterEnv1PostDriveSlider);

    // 6. Carrier 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier2_tracking", carrier2TrackingBox));
    bindSlider("carrier2_pitch", carrier2PitchSlider);
    bindSlider("carrier2_shape", carrier2ShapeSlider);
    bindSlider("carrier2_depth", carrier2DepthSlider);

    // 7. Modulator 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod2_track", mod2TrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod2_type", mod2TypeBox));
    bindSlider("mod2_shape", mod2ShapeSlider);
    bindSlider("mod2_speed", mod2SpeedSlider);

    // 8. Pitch Envelope 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pitchenv2_target", pitchEnv2TargetBox));
    bindSlider("pitchenv2_slope", pitchEnv2SlopeSlider);
    bindSlider("pitchenv2_depth", pitchEnv2DepthSlider);
    bindSlider("pitchenv2_decay", pitchEnv2DecaySlider);

    // 9. Filter 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter2_type", filter2TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter2_slope", filter2SlopeBox));
    bindSlider("filter2_cutoff", filter2CutoffSlider);
    bindSlider("filter2_resonance", filter2ResonanceSlider);

    // 10. Filter Envelope 2
    bindSlider("filterenv2_slope", filterEnv2SlopeSlider);
    bindSlider("filterenv2_depth", filterEnv2DepthSlider);
    bindSlider("filterenv2_decay", filterEnv2DecaySlider);
    bindSlider("filterenv2_postdrive", filterEnv2PostDriveSlider);

    // 11. Noise Transient
    bindSlider("noise_sh_rate", noiseShRateSlider);
    bindSlider("noise_filter", noiseFilterSlider);
    bindSlider("noise_drive", noiseDriveSlider);
    bindSlider("noise_decay", noiseDecaySlider);

    // 12. Filter 3 (Transients Filter)
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter3_type", filter3TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter3_slope", filter3SlopeBox));
    bindSlider("filter3_cutoff", filter3CutoffSlider);
    bindSlider("filter3_resonance", filter3ResonanceSlider);

    // 13. Filter Envelope 3 (Transients Filter Env)
    bindSlider("filterenv3_slope", filterEnv3SlopeSlider);
    bindSlider("filterenv3_depth", filterEnv3DepthSlider);
    bindSlider("filterenv3_decay", filterEnv3DecaySlider);
    bindSlider("filterenv3_postdrive", filterEnv3PostDriveSlider);

    // 14. Mixer
    bindSlider("mixer_carrier1_level", mixerCarrier1LevelSlider);
    bindSlider("mixer_carrier2_level", mixerCarrier2LevelSlider);
    bindSlider("mixer_ringmod", mixerRingModSlider);
    bindSlider("mixer_noise_level", mixerNoiseLevelSlider);

    // Multi-Instance FX Slot Knobs Attachments
    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            juce::String preParamId = "pre_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1);
            bindSlider(preParamId, preFXCards[s]->getKnob(p));

            juce::String postParamId = "post_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1);
            bindSlider(postParamId, postFXCards[s]->getKnob(p));
        }
    }

    // 24. Amp
    bindSlider("amp_level", ampLevelSlider);
    bindSlider("amp_pan", ampPanSlider);
    bindSlider("amp_drive", ampDriveSlider);
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "amp_limiter", ampLimiterBox));

    // 25. Amp Envelope
    bindSlider("ampenv_claps", ampEnvClapsSlider);
    bindSlider("ampenv_clapspeed", ampEnvClapSpeedSlider);
    bindSlider("ampenv_slope", ampEnvSlopeSlider);
    bindSlider("ampenv_decay", ampEnvDecaySlider);

    // 26. Pre-Amp Limiter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pre_limiter_enable", preLimiterEnableBox));
    bindSlider("pre_limiter_gain", preLimiterGainSlider);
    bindSlider("pre_limiter_thresh", preLimiterThreshSlider);
    bindSlider("pre_limiter_release", preLimiterReleaseSlider);

    // 27. Post-Amp Limiter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "post_limiter_enable", postLimiterEnableBox));
    bindSlider("post_limiter_gain", postLimiterGainSlider);
    bindSlider("post_limiter_thresh", postLimiterThreshSlider);
    bindSlider("post_limiter_release", postLimiterReleaseSlider);

    // 28. Velocity
    bindSlider("vel_slope", velSlopeSlider);
    bindSlider("vel_depth", velDepthSlider);
    bindSlider("vel_decay", velDecaySlider);
    bindSlider("vel_volume", velVolumeSlider);

    // 29. Key Tracking
    bindSlider("key_slope", keySlopeSlider);
    bindSlider("key_depth", keyDepthSlider);
    bindSlider("key_decay", keyDecaySlider);
    bindSlider("key_volume", keyVolumeSlider);

    // 30. Slop
    bindSlider("slop_freq", slopFreqSlider);
    bindSlider("slop_depth", slopDepthSlider);
    bindSlider("slop_decay", slopDecaySlider);
    bindSlider("slop_pan", slopPanSlider);

    // 31. Mod Envelopes 1..3
    bindSlider("modenv1_slope", modEnv1SlopeSlider);
    bindSlider("modenv1_depth", modEnv1DepthSlider);
    bindSlider("modenv1_decay", modEnv1DecaySlider);
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "modenv1_target", modEnv1TargetBox));

    bindSlider("modenv2_slope", modEnv2SlopeSlider);
    bindSlider("modenv2_depth", modEnv2DepthSlider);
    bindSlider("modenv2_decay", modEnv2DecaySlider);
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "modenv2_target", modEnv2TargetBox));

    bindSlider("modenv3_slope", modEnv3SlopeSlider);
    bindSlider("modenv3_depth", modEnv3DepthSlider);
    bindSlider("modenv3_decay", modEnv3DecaySlider);
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "modenv3_target", modEnv3TargetBox));

    // --- FX PICKERS SETUP & ATTACHMENTS ---
    const juce::StringArray fxChoices {
        "None",
        "Bell EQ",
        "Chorus",
        "Comb Filter",
        "Phase Smear",
        "Drive",
        "Filter",
        "Flanger",
        "Frequency Shifter",
        "Grit FX",
        "Phaser",
        "RingMod",
        "Tempo Delay",
        "Wave Folder"
    };

    for (int i = 0; i < 4; ++i) {
        preFXPickerCard->getBox(i).addItemList(fxChoices, 1);
        juce::String paramId = "pre_fx_" + juce::String(i + 1) + "_type";
        boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, paramId, preFXPickerCard->getBox(i)));
        preFXPickerCard->getBox(i).onChange = [this, i]() {
            int choice = preFXPickerCard->getBox(i).getSelectedItemIndex();
            if (choice >= 0) {
                setFXSlotDefaults(i, false, choice);
                audioProcessor.getEngine().setPreFXType(i, choice);
                preFXCards[i]->configureForType(choice);
                updatePageLayout();
                updateModTargetBoxItems();
            }
        };

        postFXPickerCard->getBox(i).addItemList(fxChoices, 1);
        juce::String postParamId = "post_fx_" + juce::String(i + 1) + "_type";
        boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, postParamId, postFXPickerCard->getBox(i)));
        postFXPickerCard->getBox(i).onChange = [this, i]() {
            int choice = postFXPickerCard->getBox(i).getSelectedItemIndex();
            if (choice >= 0) {
                setFXSlotDefaults(i, true, choice);
                audioProcessor.getEngine().setPostFXType(i, choice);
                postFXCards[i]->configureForType(choice);
                updatePageLayout();
                updateModTargetBoxItems();
            }
        };

        preFXCards[i]->configureForType(audioProcessor.getEngine().getPreFXType(i));
        postFXCards[i]->configureForType(audioProcessor.getEngine().getPostFXType(i));
    }

    auto setupCardListeners = [this](juce::Component* card, int blockId, const juce::String& name) {
        if (!card) return;
        card->addMouseListener(this, true);
        if (auto* modCard = dynamic_cast<ModuleCardComponent*>(card)) {
            modCard->onCardClicked = [this, blockId, name]() {
                if (!vizCard.getIsLocked()) {
                    vizCard.setVisualizedBlock(blockId, name);
                }
            };
        }
    };

    setupCardListeners(cardCarrier1.get(), TbdAudio::ModularDrumEngine::BLK_CARRIER1, "CARRIER 1");
    setupCardListeners(cardMod1.get(), TbdAudio::ModularDrumEngine::BLK_MODULATOR1, "MODULATOR 1");
    setupCardListeners(cardPitchEnv1.get(), TbdAudio::ModularDrumEngine::BLK_PITCHENV1, "PITCH ENV 1");
    setupCardListeners(cardFilter1.get(), TbdAudio::ModularDrumEngine::BLK_FILTER1, "FILTER 1");
    setupCardListeners(cardFilterEnv1.get(), TbdAudio::ModularDrumEngine::BLK_FILTERENV1, "FILTER ENV 1");
    setupCardListeners(cardCarrier2.get(), TbdAudio::ModularDrumEngine::BLK_CARRIER2, "CARRIER 2");
    setupCardListeners(cardMod2.get(), TbdAudio::ModularDrumEngine::BLK_MODULATOR2, "MODULATOR 2");
    setupCardListeners(cardPitchEnv2.get(), TbdAudio::ModularDrumEngine::BLK_PITCHENV2, "PITCH ENV 2");
    setupCardListeners(cardFilter2.get(), TbdAudio::ModularDrumEngine::BLK_FILTER2, "FILTER 2");
    setupCardListeners(cardFilterEnv2.get(), TbdAudio::ModularDrumEngine::BLK_FILTERENV2, "FILTER ENV 2");
    setupCardListeners(cardNoise.get(), TbdAudio::ModularDrumEngine::BLK_NOISE, "NOISE");
    setupCardListeners(cardFilter3.get(), TbdAudio::ModularDrumEngine::BLK_FILTER3, "FILTER 3");
    setupCardListeners(cardFilterEnv3.get(), TbdAudio::ModularDrumEngine::BLK_FILTERENV3, "FILTER ENV 3");
    setupCardListeners(cardMixer.get(), TbdAudio::ModularDrumEngine::BLK_MIXER, "MIXER");
    setupCardListeners(cardAmp.get(), TbdAudio::ModularDrumEngine::BLK_AMP, "AMPLIFIER");
    setupCardListeners(cardAmpEnv.get(), TbdAudio::ModularDrumEngine::BLK_AMPENV, "AMP ENV");
    setupCardListeners(cardPreLimiter.get(), TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, "PRE LIMITER");
    setupCardListeners(cardPostLimiter.get(), TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, "MASTER LIMITER");
    setupCardListeners(cardVelocity.get(), TbdAudio::ModularDrumEngine::BLK_VELOCITY, "VELOCITY");
    setupCardListeners(cardKeyTrack.get(), TbdAudio::ModularDrumEngine::BLK_KEYTRACK, "KEY TRACK");
    setupCardListeners(cardSlop.get(), TbdAudio::ModularDrumEngine::BLK_SLOP, "SLOP");
    setupCardListeners(cardModEnv1.get(), TbdAudio::ModularDrumEngine::BLK_MODENV1, "MOD ENV 1");
    setupCardListeners(cardModEnv2.get(), TbdAudio::ModularDrumEngine::BLK_MODENV2, "MOD ENV 2");
    setupCardListeners(cardModEnv3.get(), TbdAudio::ModularDrumEngine::BLK_MODENV3, "MOD ENV 3");

    for (int s = 0; s < 4; ++s) {
        if (preFXCards[s]) {
            preFXCards[s]->addMouseListener(this, true);
            preFXCards[s]->onCardClicked = [this, s]() {
                if (!vizCard.getIsLocked()) {
                    vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PRE_FX_1 + s,
                                               "PRE " + juce::String(s + 1) + ": " + preFXCards[s]->getTitle());
                }
            };
        }
        if (postFXCards[s]) {
            postFXCards[s]->addMouseListener(this, true);
            postFXCards[s]->onCardClicked = [this, s]() {
                if (!vizCard.getIsLocked()) {
                    vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_POST_FX_1 + s,
                                               "POST " + juce::String(s + 1) + ": " + postFXCards[s]->getTitle());
                }
            };
        }
    }

    scopeBuffer.resize(128, 0.0f);
    updateDynamicControls();

    setSize(1040, 740);
    setResizable(true, true);
    setResizeLimits(800, 560, 2400, 1600);

    setPage(0);
    startTimerHz(30);
}


TheKlangFarmerAudioProcessorEditor::~TheKlangFarmerAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void TheKlangFarmerAudioProcessorEditor::setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour, bool isBipolar, double defaultVal) {
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setScrollWheelEnabled(true);
    slider.setBipolar(isBipolar);
    slider.setAccentColour(trackColour);
    slider.setColour(juce::Slider::rotarySliderFillColourId, trackColour);
    slider.setColour(juce::Slider::trackColourId, trackColour);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff232733));
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff161922));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe8edf5));
    slider.setRange(0.0, 1.0, 0.0005);
    slider.setDoubleClickReturnValue(true, defaultVal);
    slider.getDefaultValue = [defaultVal]() { return defaultVal; };
}

void TheKlangFarmerAudioProcessorEditor::bindSlider(const juce::String& paramId, RotaryKnobSlider& slider) {
    slider.setParamId(paramId);
    slider.getModInfoFunc = [this](const juce::String& pid) {
        return audioProcessor.getParamModulationInfo(pid);
    };
    registeredSliders.push_back(&slider);
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramId, slider));
}

void TheKlangFarmerAudioProcessorEditor::setupBox(juce::ComboBox& box) {
    box.setVisible(false);
}

void TheKlangFarmerAudioProcessorEditor::updateCarrier1Controls() {
    int mode = carrier1TrackingBox.getSelectedItemIndex();
    if (mode == 0) { // MIDI
        cardCarrier1->setKnobLabel(0, "Offset");
        carrier1PitchSlider.setBipolar(true);
        carrier1PitchSlider.getDefaultValue = []() { return 0.5; };
    } else if (mode == 1) { // Freq
        cardCarrier1->setKnobLabel(0, "Frequency");
        carrier1PitchSlider.setBipolar(false);
        carrier1PitchSlider.getDefaultValue = []() {
            return static_cast<double>(std::log(55.0f / 20.0f) / std::log(24000.0f / 20.0f));
        };
    } else { // Note
        cardCarrier1->setKnobLabel(0, "Note");
        carrier1PitchSlider.setBipolar(false);
        carrier1PitchSlider.getDefaultValue = []() { return 33.0 / 127.0; };
    }
    carrier1PitchSlider.repaint();
    carrier1PitchSlider.updateText();
}

void TheKlangFarmerAudioProcessorEditor::updateCarrier2Controls() {
    int mode = carrier2TrackingBox.getSelectedItemIndex();
    if (mode == 0) { // MIDI
        cardCarrier2->setKnobLabel(0, "Offset");
        carrier2PitchSlider.setBipolar(true);
        carrier2PitchSlider.getDefaultValue = []() { return 0.5; };
    } else if (mode == 1) { // Freq
        cardCarrier2->setKnobLabel(0, "Frequency");
        carrier2PitchSlider.setBipolar(false);
        carrier2PitchSlider.getDefaultValue = []() {
            return static_cast<double>(std::log(55.0f / 20.0f) / std::log(24000.0f / 20.0f));
        };
    } else { // Note
        cardCarrier2->setKnobLabel(0, "Note");
        carrier2PitchSlider.setBipolar(false);
        carrier2PitchSlider.getDefaultValue = []() { return 33.0 / 127.0; };
    }
    carrier2PitchSlider.repaint();
    carrier2PitchSlider.updateText();
}

void TheKlangFarmerAudioProcessorEditor::resetToDefaults() {
    for (auto* param : audioProcessor.getParameters()) {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            rangedParam->setValueNotifyingHost(rangedParam->getDefaultValue());
        }
    }
    updateDynamicControls();
    updatePageLayout();
    repaint();
}

void TheKlangFarmerAudioProcessorEditor::bindSelector(LedSelectorComponent& selector, juce::ComboBox& box,
                                              const juce::String& paramId, const juce::StringArray& items, int numColumns) {
    box.clear();
    box.addItemList(items, 1);
    box.setVisible(false);
    selector.setItems(items, numColumns);
    selector.onChange = [this, &box, paramId](int idx) {
        box.setSelectedId(idx + 1, juce::sendNotification);
        if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
        }
    };
}

void TheKlangFarmerAudioProcessorEditor::setFXSlotDefaults(int slot, bool isPost, int fxType) {
    float defs[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    switch (fxType) {
        case 1:  defs[0] = 1.0f; defs[1] = 0.0f; defs[2] = 0.5f; defs[3] = 0.5f; break; // Bell EQ: 1kHz, 0.1 oct, 0dB, flat
        case 2:  defs[0] = 0.5398f; defs[1] = 0.60f; defs[2] = 0.60f; defs[3] = 0.50f; break; // Chorus: 1.2Hz, 60% depth, +20% fb, 50% mix
        case 3:  defs[0] = 1.0f; defs[1] = 1.0f; defs[2] = 0.5f; defs[3] = 0.75f; break; // Comb: Damp 24kHz, Cut 24kHz, Res 0%, Mix +50%:50%
        case 4:  defs[0] = 0.0f; defs[1] = 4.0f / 32.0f; defs[2] = 0.62124f; defs[3] = 0.5f; break; // Phase Smear: 2nd Order, 4 stages, 1kHz, 0%
        case 5:  defs[0] = 0.4f; defs[1] = 0.5f; defs[2] = 0.5f; defs[3] = 1.0f; break; // Drive: +6dB, 0 bias, 50% flat, Limiter On
        case 6:  defs[0] = 0.0f; defs[1] = 0.25f; defs[2] = 1.0f; defs[3] = 0.0f; break; // Filter: LPF, -12dB, 24kHz, 0% res
        case 7:  defs[0] = 0.3500f; defs[1] = 0.70f; defs[2] = 0.868f; defs[3] = 0.50f; break; // Flanger: 0.25Hz, 70% depth, +70% fb, 50% mix
        case 8:  defs[0] = 0.5f; defs[1] = TbdAudio::rangeHzToNorm(3.0f); defs[2] = 0.75f; defs[3] = 0.5f; break; // FreqShift: 0 shift, 3Hz, Blend +50%:50%, center
        case 9:  defs[0] = 1.0f; defs[1] = 1.0f; defs[2] = 0.5f; defs[3] = 0.5f; break; // Grit: 16 bit, 24kHz, 0dB, 0dB
        case 10: defs[0] = 0.4530f; defs[1] = 0.70f; defs[2] = 0.763f; defs[3] = 0.50f; break; // Phaser: 0.5Hz, 70% depth, +50% fb, 50% mix
        case 11: defs[0] = 0.0f; defs[1] = 0.50934f; defs[2] = 0.0f; defs[3] = 0.5f; break; // RingMod: sine, 1kHz, 0% amt, center
        case 12: defs[0] = 5.0f / 9.0f; defs[1] = 0.40f; defs[2] = 0.70f; defs[3] = 0.35f; break; // Tempo Delay: 1/8, 40% fb, 8kHz tone, 35% mix
        case 13: defs[0] = 1.0f; defs[1] = 0.0f; defs[2] = 0.5f; defs[3] = 0.5f; break; // WaveFolder: On, 0 fold, 0 bias, 50% flat
        default: break;
    }
    juce::String prefix = isPost ? "post_fx_" : "pre_fx_";
    for (int p = 0; p < 4; ++p) {
        juce::String paramId = prefix + juce::String(slot + 1) + "_p" + juce::String(p + 1);
        if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
            param->setValueNotifyingHost(defs[p]);
        }
    }
}

void TheKlangFarmerAudioProcessorEditor::setPage(int pageIndex) {
    if (pageIndex < 0 || pageIndex > 6) return;
    currentPage = pageIndex;
    navCard.setSelectedPage(currentPage);
    updatePageLayout();
    repaint();
}

void TheKlangFarmerAudioProcessorEditor::updatePageLayout() {
    for (int i = 0; i < 6; ++i) blankPlates[i].setVisible(false);
    if (preFXPickerCard) preFXPickerCard->setVisible(false);
    if (postFXPickerCard) postFXPickerCard->setVisible(false);

    auto hideCard = [](std::unique_ptr<ModuleCardComponent>& c) {
        if (c) c->setVisible(false);
    };

    hideCard(cardCarrier1); hideCard(cardMod1); hideCard(cardPitchEnv1);
    hideCard(cardFilter1); hideCard(cardFilterEnv1);
    hideCard(cardCarrier2); hideCard(cardMod2); hideCard(cardPitchEnv2);
    hideCard(cardFilter2); hideCard(cardFilterEnv2);
    hideCard(cardNoise); hideCard(cardFilter3); hideCard(cardFilterEnv3);
    hideCard(cardMixer);
    hideCard(cardAmp); hideCard(cardAmpEnv);
    hideCard(cardPreLimiter); hideCard(cardPostLimiter);
    hideCard(cardVelocity); hideCard(cardKeyTrack); hideCard(cardSlop);
    hideCard(cardModEnv1); hideCard(cardModEnv2); hideCard(cardModEnv3);

    for (int s = 0; s < 4; ++s) {
        if (preFXCards[s]) preFXCards[s]->setVisible(false);
        if (postFXCards[s]) postFXCards[s]->setVisible(false);
    }

    int margin = 6;
    int topOffset = 38;
    int totalW = getWidth() - 2 * margin;
    int totalH = getHeight() - topOffset - margin;
    int numCols = 4;
    int numRows = 2;
    int slotW = (totalW - (numCols - 1) * margin) / numCols;
    int slotH = (totalH - (numRows - 1) * margin) / numRows;

    auto getSlotBounds = [margin, topOffset, slotW, slotH](int slotIndex) {
        int r = slotIndex / 4;
        int c = slotIndex % 4;
        int x = margin + c * (slotW + margin);
        int y = topOffset + r * (slotH + margin);
        return juce::Rectangle<int>(x, y, slotW, slotH);
    };

    navCard.setBounds(getSlotBounds(0));
    navCard.setVisible(true);

    juce::Component* slotComponents[6] = { nullptr };

    switch (currentPage) {
        case 0: // VOICE 1
            slotComponents[0] = cardCarrier1.get();
            slotComponents[1] = cardMod1.get();
            slotComponents[2] = cardPitchEnv1.get();
            slotComponents[3] = cardFilter1.get();
            slotComponents[4] = cardFilterEnv1.get();
            slotComponents[5] = cardMixer.get();
            break;

        case 1: // VOICE 2
            slotComponents[0] = cardCarrier2.get();
            slotComponents[1] = cardMod2.get();
            slotComponents[2] = cardPitchEnv2.get();
            slotComponents[3] = cardFilter2.get();
            slotComponents[4] = cardFilterEnv2.get();
            slotComponents[5] = cardMixer.get();
            break;

        case 2: // TRANSIENTS
            slotComponents[0] = cardNoise.get();
            slotComponents[1] = &blankPlates[0];
            slotComponents[2] = &blankPlates[1];
            slotComponents[3] = cardFilter3.get();
            slotComponents[4] = cardFilterEnv3.get();
            slotComponents[5] = cardMixer.get();
            break;

        case 3: // PRE-AMP FX
            slotComponents[0] = preFXPickerCard.get();
            for (int s = 0; s < 4; ++s) {
                int t = audioProcessor.getEngine().getPreFXType(s);
                if (t > 0) {
                    slotComponents[1 + s] = preFXCards[s].get();
                } else {
                    slotComponents[1 + s] = &blankPlates[s];
                }
            }
            slotComponents[5] = cardPreLimiter.get();
            break;

        case 4: // AMPLIFIER
            slotComponents[0] = cardAmp.get();
            slotComponents[1] = cardAmpEnv.get();
            slotComponents[2] = &blankPlates[0];
            slotComponents[3] = &blankPlates[1];
            slotComponents[4] = cardPostLimiter.get();
            slotComponents[5] = cardMixer.get();
            break;

        case 5: // POST-AMP FX
            slotComponents[0] = postFXPickerCard.get();
            for (int s = 0; s < 4; ++s) {
                int t = audioProcessor.getEngine().getPostFXType(s);
                if (t > 0) {
                    slotComponents[1 + s] = postFXCards[s].get();
                } else {
                    slotComponents[1 + s] = &blankPlates[s];
                }
            }
            slotComponents[5] = cardPostLimiter.get();
            break;

        case 6: // MODULATIONS
            slotComponents[0] = cardModEnv1.get();
            slotComponents[1] = cardModEnv2.get();
            slotComponents[2] = cardModEnv3.get();
            slotComponents[3] = cardVelocity.get();
            slotComponents[4] = cardKeyTrack.get();
            slotComponents[5] = cardSlop.get();
            break;
    }

    vizCard.setBounds(getSlotBounds(4));
    vizCard.setVisible(true);

    for (int i = 0; i < 3; ++i) {
        if (slotComponents[i]) {
            slotComponents[i]->setBounds(getSlotBounds(i + 1));
            slotComponents[i]->setVisible(true);
        }
    }

    for (int i = 3; i < 6; ++i) {
        if (slotComponents[i]) {
            slotComponents[i]->setBounds(getSlotBounds(i + 2));
            slotComponents[i]->setVisible(true);
        }
    }

    if (!vizCard.getIsLocked()) {
        switch (currentPage) {
            case 0: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER1, "CARRIER 1"); break;
            case 1: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER2, "CARRIER 2"); break;
            case 2: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_NOISE, "NOISE"); break;
            case 3: {
                int blk = TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER;
                juce::String name = "PRE LIMITER";
                for (int s = 0; s < 4; ++s) {
                    if (audioProcessor.getEngine().getPreFXType(s) > 0) {
                        blk = TbdAudio::ModularDrumEngine::BLK_PRE_FX_1 + s;
                        name = "PRE " + juce::String(s + 1) + ": " + preFXCards[s]->getTitle();
                        break;
                    }
                }
                vizCard.setVisualizedBlock(blk, name);
                break;
            }
            case 4: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_AMP, "AMPLIFIER"); break;
            case 5: {
                int blk = TbdAudio::ModularDrumEngine::BLK_POST_LIMITER;
                juce::String name = "MASTER LIMITER";
                for (int s = 0; s < 4; ++s) {
                    if (audioProcessor.getEngine().getPostFXType(s) > 0) {
                        blk = TbdAudio::ModularDrumEngine::BLK_POST_FX_1 + s;
                        name = "POST " + juce::String(s + 1) + ": " + postFXCards[s]->getTitle();
                        break;
                    }
                }
                vizCard.setVisualizedBlock(blk, name);
                break;
            }
            case 6: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODENV1, "MOD ENV 1"); break;
        }
    }
}

void TheKlangFarmerAudioProcessorEditor::updateModTargetBoxItems() {
    const auto& destinations = TheKlangFarmerAudioProcessor::getModDestinations();
    auto updateBox = [&](juce::ComboBox& box) {
        for (size_t i = 0; i < destinations.size(); ++i) {
            const auto& d = destinations[i];
            int itemId = static_cast<int>(i + 2); // 1 is "None", so i=0 is itemId 2
            if (d.type == TheKlangFarmerAudioProcessor::ModTargetType::PreFX) {
                int fxType = audioProcessor.getEngine().getPreFXType(d.blockOrSlot);
                juce::String name = TheKlangFarmerAudioProcessor::getFXParamDisplayName(false, d.blockOrSlot, fxType, d.paramIndex);
                box.changeItemText(itemId, name);
            } else if (d.type == TheKlangFarmerAudioProcessor::ModTargetType::PostFX) {
                int fxType = audioProcessor.getEngine().getPostFXType(d.blockOrSlot);
                juce::String name = TheKlangFarmerAudioProcessor::getFXParamDisplayName(true, d.blockOrSlot, fxType, d.paramIndex);
                box.changeItemText(itemId, name);
            }
        }
    };

    updateBox(modEnv1TargetBox);
    updateBox(modEnv2TargetBox);
    updateBox(modEnv3TargetBox);
}

void TheKlangFarmerAudioProcessorEditor::updateDynamicControls() {
    auto syncSelector = [this](juce::ComboBox& box, LedSelectorComponent& selector, const juce::String& paramId, int& lastVal) {
        int idx = box.getSelectedItemIndex();
        if (idx < 0) {
            if (auto* param = audioProcessor.apvts.getRawParameterValue(paramId)) {
                idx = static_cast<int>(param->load());
            }
        }
        if (idx >= 0 && idx != lastVal) {
            selector.setSelectedIndex(idx, juce::dontSendNotification);
            lastVal = idx;
        }
        return idx;
    };

    int curCarrier1Track = syncSelector(carrier1TrackingBox, carrier1TrackingSelector, "carrier1_tracking", lastCarrier1Track);
    if (curCarrier1Track >= 0) {
        updateCarrier1Controls();
    }

    int curMod1Track = syncSelector(mod1TrackBox, mod1TrackSelector, "mod1_track", lastMod1Track);
    int curMod1Type  = syncSelector(mod1TypeBox, mod1TypeSelector, "mod1_type", lastMod1Type);
    if (curMod1Type >= 0 || curMod1Track >= 0) {
        if (curMod1Type == 0) {
            cardMod1->setKnobLabel(0, "Shape");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
            mod1ShapeSlider.setBipolar(false);
        } else if (curMod1Type == 1) {
            cardMod1->setKnobLabel(0, "DJ Filter");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod1ShapeSlider.setBipolar(true);
        } else {
            cardMod1->setKnobLabel(0, "S&H Rate");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod1ShapeSlider.setBipolar(false);
        }
        mod1ShapeSlider.repaint();
        mod1ShapeSlider.updateText();
        mod1SpeedSlider.updateText();
    }

    syncSelector(pitchEnv1TargetBox, pitchEnv1TargetSelector, "pitchenv1_target", lastPitchEnv1Target);
    syncSelector(filter1TypeBox, filter1TypeSelector, "filter1_type", lastFilter1Type);
    syncSelector(filter1SlopeBox, filter1SlopeSelector, "filter1_slope", lastFilter1Slope);

    int curCarrier2Track = syncSelector(carrier2TrackingBox, carrier2TrackingSelector, "carrier2_tracking", lastCarrier2Track);
    if (curCarrier2Track >= 0) {
        updateCarrier2Controls();
    }

    int curMod2Track = syncSelector(mod2TrackBox, mod2TrackSelector, "mod2_track", lastMod2Track);
    int curMod2Type  = syncSelector(mod2TypeBox, mod2TypeSelector, "mod2_type", lastMod2Type);
    if (curMod2Type >= 0 || curMod2Track >= 0) {
        if (curMod2Type == 0) {
            cardMod2->setKnobLabel(0, "Shape");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
            mod2ShapeSlider.setBipolar(false);
        } else if (curMod2Type == 1) {
            cardMod2->setKnobLabel(0, "DJ Filter");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod2ShapeSlider.setBipolar(true);
        } else {
            cardMod2->setKnobLabel(0, "S&H Rate");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod2ShapeSlider.setBipolar(false);
        }
        mod2ShapeSlider.repaint();
        mod2ShapeSlider.updateText();
        mod2SpeedSlider.updateText();
    }

    syncSelector(pitchEnv2TargetBox, pitchEnv2TargetSelector, "pitchenv2_target", lastPitchEnv2Target);
    syncSelector(filter2TypeBox, filter2TypeSelector, "filter2_type", lastFilter2Type);
    syncSelector(filter2SlopeBox, filter2SlopeSelector, "filter2_slope", lastFilter2Slope);

    syncSelector(filter3TypeBox, filter3TypeSelector, "filter3_type", lastFilter3Type);
    syncSelector(filter3SlopeBox, filter3SlopeSelector, "filter3_slope", lastFilter3Slope);

    syncSelector(ampLimiterBox, ampLimiterSelector, "amp_limiter", lastAmpLimiter);

    syncSelector(preLimiterEnableBox, preLimiterEnableSelector, "pre_limiter_enable", lastPreLimiterEnable);
    syncSelector(postLimiterEnableBox, postLimiterEnableSelector, "post_limiter_enable", lastPostLimiterEnable);

    for (int s = 0; s < 4; ++s) {
        if (preFXCards[s]) preFXCards[s]->updateDynamicControls();
        if (postFXCards[s]) postFXCards[s]->updateDynamicControls();
    }

    bool fxTypesChanged = false;
    for (int s = 0; s < 4; ++s) {
        int curPre = audioProcessor.getEngine().getPreFXType(s);
        if (curPre != lastPreFXTypes[s]) {
            lastPreFXTypes[s] = curPre;
            fxTypesChanged = true;
        }
        int curPost = audioProcessor.getEngine().getPostFXType(s);
        if (curPost != lastPostFXTypes[s]) {
            lastPostFXTypes[s] = curPost;
            fxTypesChanged = true;
        }
    }
    if (fxTypesChanged) {
        updateModTargetBoxItems();
    }
}

void TheKlangFarmerAudioProcessorEditor::timerCallback() {
    updateDynamicControls();

    for (auto* s : registeredSliders) {
        if (!s || s->getParamId().isEmpty()) continue;
        if (!s->isShowing()) continue;

        auto info = audioProcessor.getParamModulationInfo(s->getParamId());
        RotaryKnobSlider::ModulationVisual mv;
        mv.isModulated = info.isModulated;
        mv.rangeMinNorm = info.rangeMinNorm;
        mv.rangeMaxNorm = info.rangeMaxNorm;
        mv.currentNorm = info.currentNorm;
        s->setModulation(mv);
    }

    int activeBlock = vizCard.getCurrentBlockIndex();
    if (activeBlock < 0) return;

    if (activeBlock == TbdAudio::ModularDrumEngine::BLK_FILTER1) {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
        float fCutNorm = static_cast<float>(filter1CutoffSlider.getValue());
        float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
        float fRes = static_cast<float>(filter1ResonanceSlider.getValue());
        int fType = filter1TypeBox.getSelectedItemIndex();
        int fSlope = filter1SlopeBox.getSelectedItemIndex();
        vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
    } else if (activeBlock == TbdAudio::ModularDrumEngine::BLK_FILTER2) {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
        float fCutNorm = static_cast<float>(filter2CutoffSlider.getValue());
        float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
        float fRes = static_cast<float>(filter2ResonanceSlider.getValue());
        int fType = filter2TypeBox.getSelectedItemIndex();
        int fSlope = filter2SlopeBox.getSelectedItemIndex();
        vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
    } else if (activeBlock == TbdAudio::ModularDrumEngine::BLK_FILTER3) {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
        float fCutNorm = static_cast<float>(filter3CutoffSlider.getValue());
        float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
        float fRes = static_cast<float>(filter3ResonanceSlider.getValue());
        int fType = filter3TypeBox.getSelectedItemIndex();
        int fSlope = filter3SlopeBox.getSelectedItemIndex();
        vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
    } else if (activeBlock >= TbdAudio::ModularDrumEngine::BLK_PRE_FX_1 && activeBlock <= TbdAudio::ModularDrumEngine::BLK_PRE_FX_4) {
        int s = activeBlock - TbdAudio::ModularDrumEngine::BLK_PRE_FX_1;
        int fxType = audioProcessor.getEngine().getPreFXType(s);
        if (fxType == 2 && preFXCards[s]) {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
            float fCutNorm = static_cast<float>(preFXCards[s]->getKnob(2).getValue());
            float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
            float fRes = static_cast<float>(preFXCards[s]->getKnob(3).getValue());
            int fType = static_cast<int>(std::round(preFXCards[s]->getKnob(0).getValue() * 3.0f));
            int fSlope = static_cast<int>(std::round(preFXCards[s]->getKnob(1).getValue() * 4.0f));
            vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
        } else if (fxType == 9 && preFXCards[s]) {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::EqXY);
            float eqFNorm = static_cast<float>(preFXCards[s]->getKnob(0).getValue());
            float eqFHz = 20.0f * std::pow(24000.0f / 20.0f, eqFNorm);
            float eqWNorm = static_cast<float>(preFXCards[s]->getKnob(1).getValue());
            float eqWOct = 0.1f * std::pow(100.0f, eqWNorm);
            float eqGNorm = static_cast<float>(preFXCards[s]->getKnob(2).getValue());
            float eqGDb = (eqGNorm - 0.5f) * 48.0f;
            float eqDjNorm = static_cast<float>(preFXCards[s]->getKnob(3).getValue());
            vizCard.getOscilloscope().updateEqParams(eqFHz, eqWOct, eqGDb, eqDjNorm);
        } else {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::Oscilloscope);
            audioProcessor.getEngine().getScopeData(activeBlock, scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
            vizCard.getOscilloscope().updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
        }
    } else if (activeBlock >= TbdAudio::ModularDrumEngine::BLK_POST_FX_1 && activeBlock <= TbdAudio::ModularDrumEngine::BLK_POST_FX_4) {
        int s = activeBlock - TbdAudio::ModularDrumEngine::BLK_POST_FX_1;
        int fxType = audioProcessor.getEngine().getPostFXType(s);
        if (fxType == 2 && postFXCards[s]) {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
            float fCutNorm = static_cast<float>(postFXCards[s]->getKnob(2).getValue());
            float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
            float fRes = static_cast<float>(postFXCards[s]->getKnob(3).getValue());
            int fType = static_cast<int>(std::round(postFXCards[s]->getKnob(0).getValue() * 3.0f));
            int fSlope = static_cast<int>(std::round(postFXCards[s]->getKnob(1).getValue() * 4.0f));
            vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
        } else if (fxType == 9 && postFXCards[s]) {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::EqXY);
            float eqFNorm = static_cast<float>(postFXCards[s]->getKnob(0).getValue());
            float eqFHz = 20.0f * std::pow(24000.0f / 20.0f, eqFNorm);
            float eqWNorm = static_cast<float>(postFXCards[s]->getKnob(1).getValue());
            float eqWOct = 0.1f * std::pow(100.0f, eqWNorm);
            float eqGNorm = static_cast<float>(postFXCards[s]->getKnob(2).getValue());
            float eqGDb = (eqGNorm - 0.5f) * 48.0f;
            float eqDjNorm = static_cast<float>(postFXCards[s]->getKnob(3).getValue());
            vizCard.getOscilloscope().updateEqParams(eqFHz, eqWOct, eqGDb, eqDjNorm);
        } else {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::Oscilloscope);
            audioProcessor.getEngine().getScopeData(activeBlock, scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
            vizCard.getOscilloscope().updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
        }
    } else {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::Oscilloscope);
        audioProcessor.getEngine().getScopeData(activeBlock, scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
        vizCard.getOscilloscope().updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
    }
}

void TheKlangFarmerAudioProcessorEditor::paint(juce::Graphics& g) {
    juce::ColourGradient bgGrad(juce::Colour(0xff12141a), 0, 0,
                                juce::Colour(0xff0a0b0e), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    g.setColour(juce::Colour(0xff171a22));
    g.fillRect(0, 0, getWidth(), 36);

    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(36, 0.0f, static_cast<float>(getWidth()));

    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("THE KLANG FARMER", 14, 0, 180, 36, juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff4a9eff));
#ifdef JucePlugin_VersionString
    juce::String verStr = "v" JucePlugin_VersionString;
#else
    juce::String verStr = "v0.1.4";
#endif
    g.drawText(verStr, 196, 0, 48, 36, juce::Justification::centredLeft);

    int subtitleWidth = juce::jmax(0, getWidth() - 350 - 250);
    g.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff75849b));
    g.drawText("PAGED MODULAR DUAL FM SYNTHESIS DRUM VOICE", 250, 0, subtitleWidth, 36, juce::Justification::centredLeft);
}

void TheKlangFarmerAudioProcessorEditor::resized() {
    guideButton.setBounds(getWidth() - 346, 5, 90, 26);
    initButton.setBounds(getWidth() - 246, 5, 90, 26);
    triggerButton.setBounds(getWidth() - 146, 5, 136, 26);
    quickstartGuide.setBounds(getLocalBounds());

    updatePageLayout();
}

void TheKlangFarmerAudioProcessorEditor::handleCardInteraction(juce::Component* comp) {
    if (vizCard.getIsLocked()) return;
    if (comp == nullptr) return;

    auto isInside = [](juce::Component* c, juce::Component* target) {
        if (!target) return false;
        while (c != nullptr) {
            if (c == target) return true;
            c = c->getParentComponent();
        }
        return false;
    };

    if (isInside(comp, cardCarrier1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER1, "CARRIER 1");
    } else if (isInside(comp, cardMod1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, "MODULATOR 1");
    } else if (isInside(comp, cardPitchEnv1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, "PITCH ENV 1");
    } else if (isInside(comp, cardFilter1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTER1, "FILTER 1");
    } else if (isInside(comp, cardFilterEnv1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, "FILTER ENV 1");
    } else if (isInside(comp, cardCarrier2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER2, "CARRIER 2");
    } else if (isInside(comp, cardMod2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, "MODULATOR 2");
    } else if (isInside(comp, cardPitchEnv2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, "PITCH ENV 2");
    } else if (isInside(comp, cardFilter2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTER2, "FILTER 2");
    } else if (isInside(comp, cardFilterEnv2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, "FILTER ENV 2");
    } else if (isInside(comp, cardNoise.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_NOISE, "NOISE");
    } else if (isInside(comp, cardFilter3.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTER3, "FILTER 3");
    } else if (isInside(comp, cardFilterEnv3.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, "FILTER ENV 3");
    } else if (isInside(comp, cardMixer.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MIXER, "MIXER");
    } else if (isInside(comp, cardAmp.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_AMP, "AMPLIFIER");
    } else if (isInside(comp, cardAmpEnv.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_AMPENV, "AMP ENV");
    } else if (isInside(comp, cardPreLimiter.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, "PRE LIMITER");
    } else if (isInside(comp, cardPostLimiter.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, "MASTER LIMITER");
    } else if (isInside(comp, cardVelocity.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_VELOCITY, "VELOCITY");
    } else if (isInside(comp, cardKeyTrack.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, "KEY TRACK");
    } else if (isInside(comp, cardSlop.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_SLOP, "SLOP");
    } else if (isInside(comp, cardModEnv1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODENV1, "MOD ENV 1");
    } else if (isInside(comp, cardModEnv2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODENV2, "MOD ENV 2");
    } else if (isInside(comp, cardModEnv3.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODENV3, "MOD ENV 3");
    } else {
        for (int s = 0; s < 4; ++s) {
            if (isInside(comp, preFXCards[s].get())) {
                vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PRE_FX_1 + s,
                                           "PRE " + juce::String(s + 1) + ": " + preFXCards[s]->getTitle());
                return;
            }
            if (isInside(comp, postFXCards[s].get())) {
                vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_POST_FX_1 + s,
                                           "POST " + juce::String(s + 1) + ": " + postFXCards[s]->getTitle());
                return;
            }
        }
    }
}

void TheKlangFarmerAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    handleCardInteraction(e.eventComponent);
}

void TheKlangFarmerAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e) {
    handleCardInteraction(e.eventComponent);
}

void TheKlangFarmerAudioProcessorEditor::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& /*d*/) {
    handleCardInteraction(e.eventComponent);
}


