#include "UIComponents.h"
#include "ParameterManager.h"

// --- SAFE PARSING & FORMATTING HELPERS ---

double parseNumberSafe(const juce::String& text, double fallback) {
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

juce::String getMidiNoteName(int noteNumber) {
    noteNumber = std::clamp(noteNumber, 0, 127);
    const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (noteNumber / 12) - 1;
    return juce::String(names[noteNumber % 12]) + juce::String(octave) + " [" + juce::String(noteNumber) + "]";
}

// Formatters and parsers
juce::String formatMidiNote(double val) {
    int note = static_cast<int>(std::round(val * 127.0));
    return getMidiNoteName(note);
}
double parseMidiNote(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 36.0) / 127.0, 0.0, 1.0);
}

juce::String formatSemi(double val) {
    float semi = static_cast<float>((val - 0.5) * 48.0);
    return juce::String(semi, 1) + " st";
}
double parseSemi(const juce::String& text) {
    double semi = parseNumberSafe(text, 0.0);
    return std::clamp((semi / 48.0) + 0.5, 0.0, 1.0);
}

juce::String formatSemi24(double val) {
    int semi = static_cast<int>(std::round((val - 0.5) * 48.0));
    return (semi > 0 ? "+" : "") + juce::String(semi) + " st";
}
double parseSemi24(const juce::String& text) {
    double semi = parseNumberSafe(text, 0.0);
    return std::clamp((semi / 48.0) + 0.5, 0.0, 1.0);
}

juce::String formatCarrierFreqHz(double val) {
    float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    if (hz >= 100.0f) return juce::String(hz, 1) + " Hz";
    return juce::String(hz, 2) + " Hz";
}
double parseCarrierFreqHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 55.0);
    hz = std::clamp(hz, 20.0, 24000.0);
    return std::log(hz / 20.0) / std::log(24000.0 / 20.0);
}

juce::String formatNoteDetail(double val) {
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

double parseNoteDetail(const juce::String& text) {
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

juce::String formatRatio(double val) {
    if (val <= 0.5) {
        float denom = 32.0f - static_cast<float>(val * 2.0) * 31.0f;
        if (std::abs(denom - 1.0f) < 0.05f) return "1:1";
        return "1:" + juce::String(denom, 1);
    } else {
        float num = 1.0f + static_cast<float>((val - 0.5) * 2.0) * 31.0f;
        if (std::abs(num - 1.0f) < 0.05f) return "1:1";
        return juce::String(num, 1) + ":1";
    }
}

double parseRatio(const juce::String& text) {
    juce::String t = text.trim();
    if (t.containsChar(':')) {
        auto parts = juce::StringArray::fromTokens(t, ":", "");
        if (parts.size() >= 2) {
            double left = parseNumberSafe(parts[0], 1.0);
            double right = parseNumberSafe(parts[1], 1.0);
            if (right <= 0.0) right = 1.0;
            double ratio = left / right;
            if (ratio <= 1.0) {
                double denom = (ratio > 0.0) ? (1.0 / ratio) : 32.0;
                denom = std::clamp(denom, 1.0, 32.0);
                return std::clamp((32.0 - denom) / 62.0, 0.0, 0.5);
            } else {
                ratio = std::clamp(ratio, 1.0, 32.0);
                return std::clamp(0.5 + (ratio - 1.0) / 62.0, 0.5, 1.0);
            }
        }
    }
    double r = parseNumberSafe(t, 1.0);
    if (r <= 0.0) return 0.0;
    if (r <= 1.0) {
        double denom = 1.0 / r;
        denom = std::clamp(denom, 1.0, 32.0);
        return std::clamp((32.0 - denom) / 62.0, 0.0, 0.5);
    } else {
        r = std::clamp(r, 1.0, 32.0);
        return std::clamp(0.5 + (r - 1.0) / 62.0, 0.5, 1.0);
    }
}

juce::String formatAmpDriveDb(double val) {
    if (val < 0.001) return "-inf dB";
    if (std::abs(val - 0.5) < 0.002) return "0.0 dB";
    if (val < 0.5) {
        float db = static_cast<float>((val / 0.5 - 1.0) * 60.0);
        return juce::String(db, 1) + " dB";
    }
    float db = static_cast<float>(((val - 0.5) / 0.5) * 24.0);
    return "+" + juce::String(db, 1) + " dB";
}

double parseAmpDriveDb(const juce::String& text) {
    juce::String t = text.trim();
    if (t.containsIgnoreCase("-inf") || t.containsIgnoreCase("inf")) return 0.0;
    double db = parseNumberSafe(t, 0.0);
    if (std::abs(db) < 0.01) return 0.5;
    if (db < 0.0) {
        db = std::clamp(db, -60.0, 0.0);
        return std::clamp((db / 60.0 + 1.0) * 0.5, 0.0, 0.5);
    }
    db = std::clamp(db, 0.0, 24.0);
    return std::clamp(0.5 + (db / 24.0) * 0.5, 0.5, 1.0);
}

juce::String formatVelocityFloor(double val) {
    if (val <= 0.0) return "1%";
    if (val >= 1.0) return "100%";
    if (std::abs(val - 0.5) < 0.01) return "50%";
    float pct = (val <= 0.5)
        ? static_cast<float>(1.0 + val * 98.0)
        : static_cast<float>(50.0 + (val - 0.5) * 100.0);
    return juce::String(static_cast<int>(std::round(pct))) + "%";
}

double parseVelocityFloor(const juce::String& text) {
    double p = parseNumberSafe(text, 50.0);
    p = std::clamp(p, 1.0, 100.0);
    if (p <= 50.0) {
        return std::clamp((p - 1.0) / 98.0, 0.0, 0.5);
    }
    return std::clamp(0.5 + (p - 50.0) / 100.0, 0.5, 1.0);
}

juce::String formatPercent(double val) {
    return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
}
double parsePercent(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 50.0) / 100.0, 0.0, 1.0);
}

juce::String formatBipolarPercent(double val) {
    int p = static_cast<int>(std::round((val - 0.5) * 200.0));
    return (p > 0 ? "+" : "") + juce::String(p) + "%";
}
double parseBipolarPercent(const juce::String& text) {
    double p = parseNumberSafe(text, 0.0);
    return std::clamp((p / 200.0) + 0.5, 0.0, 1.0);
}

juce::String formatTimeMs(double val) {
    float ms = 5.0f * std::pow(60000.0f / 5.0f, static_cast<float>(val));
    if (ms >= 1000.0f) return juce::String(ms / 1000.0f, 2) + " s";
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
double parseTimeMs(const juce::String& text) {
    double ms = parseNumberSafe(text, 333.0);
    if (text.containsIgnoreCase("s") && !text.containsIgnoreCase("ms")) ms *= 1000.0;
    ms = std::clamp(ms, 5.0, 60000.0);
    return std::log(ms / 5.0) / std::log(60000.0 / 5.0);
}

juce::String formatNoiseTimeMs(double val) {
    float ms = 1.0f * std::pow(60000.0f / 1.0f, static_cast<float>(val));
    if (ms >= 1000.0f) return juce::String(ms / 1000.0f, 2) + " s";
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
double parseNoiseTimeMs(const juce::String& text) {
    double ms = parseNumberSafe(text, 100.0);
    if (text.containsIgnoreCase("s") && !text.containsIgnoreCase("ms")) ms *= 1000.0;
    ms = std::clamp(ms, 1.0, 60000.0);
    return std::log(ms / 1.0) / std::log(60000.0 / 1.0);
}

juce::String formatFreqHz(double val) {
    float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    if (hz >= 10.0f) return juce::String(hz, 1) + " Hz";
    return juce::String(hz, 2) + " Hz";
}
double parseFreqHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 1000.0);
    hz = std::clamp(hz, 0.1, 24000.0);
    return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
}

juce::String formatEqFreqHz(double val) {
    float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    return juce::String(static_cast<int>(std::round(hz))) + " Hz";
}
double parseEqFreqHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 1000.0);
    hz = std::clamp(hz, 20.0, 24000.0);
    return std::log(hz / 20.0) / std::log(24000.0 / 20.0);
}

juce::String formatDb(double val) {
    float db = -6.0f + static_cast<float>(val) * 30.0f;
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
double parseDb(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 6.0) / 30.0, 0.0, 1.0);
}

juce::String formatBipolarDb(double val) {
    float db = static_cast<float>((val - 0.5) * 48.0);
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
double parseBipolarDb(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db / 48.0) + 0.5, 0.0, 1.0);
}


juce::String formatWaveshape(double val) {
    if (val < 0.1) return juce::String("Sine");
    if (val < 0.3) return juce::String("Triangle");
    if (val < 0.5) return juce::String("Sawtooth");
    if (val < 0.8) return juce::String("Square");
    return juce::String("PWM");
}

double parseWaveshape(const juce::String& text) {
    if (text.containsIgnoreCase("sin")) return 0.0;
    if (text.containsIgnoreCase("tri")) return 0.2;
    if (text.containsIgnoreCase("saw")) return 0.4;
    if (text.containsIgnoreCase("sq") || text.containsIgnoreCase("pul")) return 0.6;
    if (text.containsIgnoreCase("pwm")) return 1.0;
    return 0.0;
}

juce::String formatWetDry(double val) {
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

double parseWetDry(const juce::String& text) {
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

juce::String formatMixerLevel(double val) {
    float pct = (val <= 0.5) ? static_cast<float>(val * 200.0) : static_cast<float>(100.0 + (val - 0.5) * 600.0);
    return juce::String(static_cast<int>(std::round(pct))) + "%";
}
double parseMixerLevel(const juce::String& text) {
    double pct = parseNumberSafe(text, 100.0);
    pct = std::clamp(pct, 0.0, 400.0);
    if (pct <= 100.0) return pct / 200.0;
    return 0.5 + (pct - 100.0) / 600.0;
}

juce::String formatOctaves(double val) {
    float oct = static_cast<float>((val - 0.5) * 10.0);
    return (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct";
}
double parseOctaves(const juce::String& text) {
    double oct = parseNumberSafe(text, 0.0);
    return std::clamp((oct / 10.0) + 0.5, 0.0, 1.0);
}

juce::String formatFilterOctaves(double val) {
    float oct = static_cast<float>((val - 0.5) * 20.0);
    return (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct";
}
double parseFilterOctaves(const juce::String& text) {
    double oct = parseNumberSafe(text, 0.0);
    return std::clamp((oct / 20.0) + 0.5, 0.0, 1.0);
}

juce::String formatBits(double val) {
    float b = 1.0f + static_cast<float>(val) * 15.0f;
    return juce::String(b, 1) + " bit";
}
double parseBits(const juce::String& text) {
    double b = parseNumberSafe(text, 16.0);
    return std::clamp((b - 1.0) / 15.0, 0.0, 1.0);
}

juce::String formatWavefolds(double val) {
    float f = static_cast<float>(val) * 8.0f;
    return juce::String(f, 2);
}
double parseWavefolds(const juce::String& text) {
    double f = parseNumberSafe(text, 0.0);
    return std::clamp(f / 8.0, 0.0, 1.0);
}

juce::String formatStages(double val) {
    int s = static_cast<int>(std::round(val * 32.0));
    return juce::String(s);
}
double parseStages(const juce::String& text) {
    double s = parseNumberSafe(text, 4.0);
    return std::clamp(s / 32.0, 0.0, 1.0);
}

juce::String formatClaps(double val) {
    int c = static_cast<int>(std::round(val * 32.0));
    return juce::String(c);
}
double parseClaps(const juce::String& text) {
    double c = parseNumberSafe(text, 0.0);
    return std::clamp(c / 32.0, 0.0, 1.0);
}

juce::String formatClapSpeed(double val) {
    float ms = 1.0f + static_cast<float>(val) * 14.0f;
    return juce::String(ms, 1) + " ms";
}
double parseClapSpeed(const juce::String& text) {
    double ms = parseNumberSafe(text, 3.0);
    return std::clamp((ms - 1.0) / 14.0, 0.0, 1.0);
}

juce::String formatLimiterGain(double val) {
    float db = -12.0f + static_cast<float>(val) * 36.0f;
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
double parseLimiterGain(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 12.0) / 36.0, 0.0, 1.0);
}

juce::String formatLimiterThresh(double val) {
    float db = -24.0f + static_cast<float>(val) * 24.0f;
    return juce::String(db, 1) + " dB";
}
double parseLimiterThresh(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 24.0) / 24.0, 0.0, 1.0);
}

juce::String formatLimiterRelease(double val) {
    float ms = 1.0f * std::pow(500.0f / 1.0f, static_cast<float>(val));
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
double parseLimiterRelease(const juce::String& text) {
    double ms = parseNumberSafe(text, 50.0);
    ms = std::clamp(ms, 1.0, 500.0);
    return std::log(ms / 1.0) / std::log(500.0 / 1.0);
}

juce::String formatSlop(double val) {
    return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
}
double parseSlop(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 0.0) / 100.0, 0.0, 1.0);
}

juce::String formatVelocitySlope(double val) {
    if (val < 0.70) return "EXP";
    if (val > 0.80) return "LOG";
    return "LIN";
}
double parseVelocitySlope(const juce::String& text) {
    if (text.containsIgnoreCase("exp")) return 0.55125;
    if (text.containsIgnoreCase("log")) return 1.0;
    return 0.75;
}

juce::String formatSlope(double val) {
    if (val < 0.70) return "EXP";
    if (val > 0.80) return "LOG";
    return "LIN";
}
double parseSlope(const juce::String& text) {
    if (text.containsIgnoreCase("exp")) return 0.55125;
    if (text.containsIgnoreCase("log")) return 1.0;
    return 0.75;
}

juce::String formatEqWidthOct(double val) {
    float oct = 0.1f * std::pow(100.0f, static_cast<float>(val));
    return juce::String(oct, 2) + " oct";
}
double parseEqWidthOct(const juce::String& text) {
    double oct = parseNumberSafe(text, 0.1);
    oct = std::clamp(oct, 0.1, 10.0);
    return std::log(oct / 0.1) / std::log(100.0);
}

juce::String formatRangeHz(double val) {
    float hz = TbdAudio::normToRangeHz(static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    return juce::String(hz, 1) + " Hz";
}
double parseRangeHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 3.0);
    return static_cast<double>(TbdAudio::rangeHzToNorm(static_cast<float>(hz)));
}

juce::String formatChorusRate(double val) {
    float hz = 0.1f * std::pow(100.0f, static_cast<float>(val));
    return juce::String(hz, (hz < 1.0f ? 2 : 1)) + " Hz";
}
double parseChorusRate(const juce::String& text) {
    double hz = parseNumberSafe(text, 1.2);
    hz = std::clamp(hz, 0.1, 10.0);
    return std::clamp(std::log(hz / 0.1) / std::log(100.0), 0.0, 1.0);
}

juce::String formatPhaserRate(double val) {
    float hz = 0.05f * std::pow(160.0f, static_cast<float>(val));
    return juce::String(hz, (hz < 1.0f ? 2 : 1)) + " Hz";
}
double parsePhaserRate(const juce::String& text) {
    double hz = parseNumberSafe(text, 0.5);
    hz = std::clamp(hz, 0.05, 8.0);
    return std::clamp(std::log(hz / 0.05) / std::log(160.0), 0.0, 1.0);
}

juce::String formatFlangerRate(double val) {
    float hz = 0.05f * std::pow(100.0f, static_cast<float>(val));
    return juce::String(hz, (hz < 1.0f ? 2 : 1)) + " Hz";
}
double parseFlangerRate(const juce::String& text) {
    double hz = parseNumberSafe(text, 0.25);
    hz = std::clamp(hz, 0.05, 5.0);
    return std::clamp(std::log(hz / 0.05) / std::log(100.0), 0.0, 1.0);
}

juce::String formatDelayDiv(double val) {
    const juce::String names[] = { "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4", "1/4D", "1/2" };
    int idx = std::clamp(static_cast<int>(std::round(val * 9.0)), 0, 9);
    return names[idx];
}
double parseDelayDiv(const juce::String& text) {
    const juce::String names[] = { "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4", "1/4D", "1/2" };
    for (int i = 0; i < 10; ++i) {
        if (text.trim().equalsIgnoreCase(names[i])) return static_cast<double>(i) / 9.0;
    }
    return 5.0 / 9.0;
}

juce::String formatDelayTone(double val) {
    float hz = 500.0f * std::pow(40.0f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 1) + " kHz";
    return juce::String(static_cast<int>(std::round(hz))) + " Hz";
}
double parseDelayTone(const juce::String& text) {
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

juce::Rectangle<int> RotaryKnobLookAndFeel::getTooltipBounds(const juce::String& tipText,
                                                             juce::Point<int> screenPos,
                                                             juce::Rectangle<int> parentArea)
{
    juce::AttributedString as;
    as.append(tipText, juce::Font(juce::FontOptions(12.5f, juce::Font::bold)), juce::Colour(0xfff0f6fc));
    as.setWordWrap(juce::AttributedString::WordWrap::byWord);

    juce::TextLayout tl;
    tl.createLayout(as, 280.0f);

    int contentW = static_cast<int>(std::ceil(tl.getWidth()));
    int contentH = static_cast<int>(std::ceil(tl.getHeight()));

    int w = contentW + 20;
    int h = contentH + 14;

    int x = (screenPos.x > parentArea.getCentreX()) ? (screenPos.x - w - 12) : (screenPos.x + 16);
    int y = (screenPos.y > parentArea.getCentreY()) ? (screenPos.y - h - 12) : (screenPos.y + 16);

    return juce::Rectangle<int>(x, y, w, h).constrainedWithin(parentArea.reduced(6));
}

void RotaryKnobLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height)
{
    juce::Rectangle<float> bounds(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    auto box = bounds.reduced(1.0f);

    g.setColour(juce::Colour(0xff101722));
    g.fillRoundedRectangle(box, 4.0f);

    g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.18f));
    g.drawRoundedRectangle(box, 4.0f, 2.5f);

    g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.75f));
    g.drawRoundedRectangle(box, 4.0f, 1.0f);

    juce::AttributedString as;
    as.append(text, juce::Font(juce::FontOptions(12.5f, juce::Font::bold)), juce::Colour(0xfff0f6fc));
    as.setWordWrap(juce::AttributedString::WordWrap::byWord);

    juce::TextLayout tl;
    tl.createLayout(as, static_cast<float>(width - 20));
    tl.draw(g, juce::Rectangle<float>(10.0f, 7.0f, static_cast<float>(width - 20), static_cast<float>(height - 14)));
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
        if (filterType == 3) {
            constexpr float minBrfQ = 0.25f;
            qVal = minBrfQ + filterResonance * (18.0f - minBrfQ);
            if (filterSlope == 0) qVal = minBrfQ + filterResonance * 5.0f;
        }

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
                float singleMag = std::abs(1.0f - r * r) / std::max(d, 1e-4f);
                if (filterSlope <= 1) mag = singleMag;
                else if (filterSlope == 2) mag = singleMag;
                else if (filterSlope == 3) mag = singleMag * singleMag;
                else                       mag = singleMag * singleMag * singleMag;
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
    if (auto* top = getTopLevelComponent()) {
        for (int i = 0; i < top->getNumChildComponents(); ++i) {
            if (auto* tw = dynamic_cast<juce::TooltipWindow*>(top->getChildComponent(i))) {
                tw->hideTip();
                break;
            }
        }
    }

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

    if (auto* top = getTopLevelComponent()) {
        for (int i = 0; i < top->getNumChildComponents(); ++i) {
            if (auto* tw = dynamic_cast<juce::TooltipWindow*>(top->getChildComponent(i))) {
                tw->hideTip();
                break;
            }
        }
    }

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
                                               std::function<ParamModulationInfo(const juce::String&)> modGetter)
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

    if (auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId)) {
        juce::Logger::writeToLog("SliderCalloutComponent paramId: " + paramId + " snapPoints: " + juce::String(def->snapPoints.size()));
        for (const auto& poi : def->snapPoints) {
            auto* btn = new juce::TextButton(poi.label);
            btn->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a3242));
            btn->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffedf2fa));
            btn->onClick = [this, val = poi.value]() {
                slider.setValue(val, juce::sendNotificationAsync);
                if (auto* callout = findParentComponentOfClass<juce::CallOutBox>()) {
                    callout->dismiss();
                }
            };
            addAndMakeVisible(presetButtons.add(btn));
        }
    }

    if (!presetButtons.isEmpty()) {
        h += 28;
    }

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
        g.drawText(RlyehSound::ParameterManager::getInstance().getGlobalString("active_modulation", "ACTIVE MODULATION"), 10.0f, curY, bounds.getWidth() - 20.0f, 14.0f, juce::Justification::centredLeft, true);
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
    int curY = 26;
    if (!presetButtons.isEmpty()) {
        int gap = 4;
        int totalWidth = getWidth() - 16;
        int btnW = (totalWidth - gap * (presetButtons.size() - 1)) / presetButtons.size();
        int curX = 8;
        for (auto* btn : presetButtons) {
            btn->setBounds(curX, curY, btnW, 20);
            curX += btnW + gap;
        }
        curY += 28;
    }
    editor.setBounds(8, curY, getWidth() - 16, 24);
}

void RotaryKnobSlider::openHoveringEditor() {
    if (auto* top = getTopLevelComponent()) {
        for (int i = 0; i < top->getNumChildComponents(); ++i) {
            if (auto* tw = dynamic_cast<juce::TooltipWindow*>(top->getChildComponent(i))) {
                tw->hideTip();
                break;
            }
        }
    }
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
        if (modulation.showNeedle) {
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
    itemStyles.assign(static_cast<size_t>(items.size()), std::nullopt);
    repaint();
}

void LedSelectorComponent::setItemStyle(int index, const ItemStyle& style) {
    if (juce::isPositiveAndBelow(index, items.size())) {
        if (itemStyles.size() < static_cast<size_t>(items.size()))
            itemStyles.resize(static_cast<size_t>(items.size()), std::nullopt);
        itemStyles[static_cast<size_t>(index)] = style;
        repaint();
    }
}

void LedSelectorComponent::clearItemStyles() {
    std::fill(itemStyles.begin(), itemStyles.end(), std::nullopt);
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

    const juce::Colour baseIdleBorder(0xff2b3140);
    const juce::Colour baseHoverBorder(0xff3f495e);
    const juce::Colour baseIdleText(0xffb0bdd0);
    const juce::Colour baseHoverText(0xffe6edf8);

    for (int i = 0; i < n; ++i) {
        auto r = getItemBounds(i).toFloat().reduced(2.0f, 1.0f);
        bool isSel = (i == selectedIndex);
        bool isHov = (i == hoveredIndex);

        const bool hasCustom = (static_cast<size_t>(i) < itemStyles.size() && itemStyles[static_cast<size_t>(i)].has_value());
        const juce::Colour primaryCol = hasCustom ? itemStyles[static_cast<size_t>(i)]->primaryAccent : accent;
        const bool hasSecondary = hasCustom && itemStyles[static_cast<size_t>(i)]->secondaryAccent.has_value();
        const juce::Colour secondaryCol = hasSecondary ? *itemStyles[static_cast<size_t>(i)]->secondaryAccent : primaryCol;
        const bool hasCustomText = hasCustom && itemStyles[static_cast<size_t>(i)]->textColour.has_value();

        // Tactile button background & border
        if (isSel) {
            if (hasSecondary) {
                juce::ColourGradient grad(secondaryCol.withAlpha(0.22f), r.getX(), r.getY(),
                                          primaryCol.withAlpha(0.22f), r.getRight(), r.getBottom(), false);
                g.setGradientFill(grad);
            } else {
                juce::ColourGradient grad(primaryCol.withAlpha(0.24f), r.getX(), r.getY(),
                                          primaryCol.withAlpha(0.10f), r.getX(), r.getBottom(), false);
                g.setGradientFill(grad);
            }
            g.fillRoundedRectangle(r, 4.0f);
            g.setColour(primaryCol.withAlpha(0.85f));
            g.drawRoundedRectangle(r, 4.0f, 1.2f);
        } else if (isHov) {
            g.setColour(juce::Colour(0xff222733));
            g.fillRoundedRectangle(r, 4.0f);
            g.setColour(hasCustom ? baseHoverBorder.interpolatedWith(primaryCol, 0.50f) : baseHoverBorder);
            g.drawRoundedRectangle(r, 4.0f, 1.0f);
        } else {
            g.setColour(juce::Colour(0xff181b23));
            g.fillRoundedRectangle(r, 4.0f);
            g.setColour(hasCustom ? baseIdleBorder.interpolatedWith(primaryCol, 0.28f) : baseIdleBorder);
            g.drawRoundedRectangle(r, 4.0f, 1.0f);
        }

        // LED dot indicator
        float ledSize = 6.0f;
        float ledX = r.getX() + 4.5f;
        float ledY = r.getCentreY() - ledSize * 0.5f;
        auto ledBounds = juce::Rectangle<float>(ledX, ledY, ledSize, ledSize);

        if (isSel) {
            if (hasSecondary) {
                // Dual LED halo: outer secondary glow + inner primary ring + gradient core
                g.setColour(secondaryCol.withAlpha(0.45f));
                g.fillEllipse(ledBounds.expanded(2.8f));
                g.setColour(primaryCol.withAlpha(0.55f));
                g.fillEllipse(ledBounds.expanded(1.4f));

                juce::ColourGradient ledGrad(secondaryCol, ledBounds.getTopLeft(),
                                             primaryCol, ledBounds.getBottomRight(), false);
                g.setGradientFill(ledGrad);
                g.fillEllipse(ledBounds);
            } else {
                g.setColour(primaryCol.withAlpha(0.40f));
                g.fillEllipse(ledBounds.expanded(2.0f));
                g.setColour(primaryCol);
                g.fillEllipse(ledBounds);
            }
            g.setColour(juce::Colours::white);
            g.fillEllipse(ledBounds.reduced(1.2f));
        } else {
            g.setColour(juce::Colour(0xff20242e));
            g.fillEllipse(ledBounds);
            juce::Colour idleLedRing(0xff353d4c);
            g.setColour(hasCustom ? idleLedRing.interpolatedWith(secondaryCol, 0.35f) : idleLedRing);
            g.drawEllipse(ledBounds, 0.8f);
        }

        auto textBounds = r.withTrimmedLeft(14.0f).withTrimmedRight(2.0f);
        g.setFont(juce::FontOptions(isSel ? 13.5f : 13.0f, juce::Font::bold));

        if (hasCustomText) {
            juce::Colour customTextCol = *itemStyles[static_cast<size_t>(i)]->textColour;
            if (isSel) {
                g.setColour(customTextCol.brighter(0.25f));
            } else if (isHov) {
                g.setColour(baseHoverText.interpolatedWith(customTextCol, 0.55f));
            } else {
                g.setColour(baseIdleText.interpolatedWith(customTextCol, 0.38f));
            }
        } else {
            g.setColour(isSel ? juce::Colours::white : (isHov ? baseHoverText : baseIdleText));
        }

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

void LedSelectorComponent::setItemTooltips(const juce::StringArray& tooltips) {
    itemTooltips = tooltips;
}

void LedSelectorComponent::setItemTooltip(int index, const juce::String& tooltip) {
    while (itemTooltips.size() <= index) {
        itemTooltips.add(juce::String());
    }
    itemTooltips.set(index, tooltip);
}

juce::String LedSelectorComponent::getTooltip() {
    if (hoveredIndex >= 0 && hoveredIndex < itemTooltips.size() && itemTooltips[hoveredIndex].isNotEmpty()) {
        return itemTooltips[hoveredIndex];
    }
    return juce::SettableTooltipClient::getTooltip();
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
            knobs[i]->setLightTrough(customKnobLightTrough[i].value_or(knobsLightTrough));
        }
    }
    repaint();
}

void ModuleCardComponent::setPanelStyle(PanelStyle style) {
    panelStyle = style;
    for (int i = 0; i < 4; ++i) {
        if (knobs[i]) {
            knobs[i]->setLightTrough(customKnobLightTrough[i].value_or(knobsLightTrough));
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

void ModuleCardComponent::setKnob(int slotIndex, const juce::String& label, RotaryKnobSlider* slider,
                                  std::optional<juce::Colour> customAccent,
                                  std::optional<bool> customLightTrough) {
    if (slotIndex >= 0 && slotIndex < 4) {
        knobs[slotIndex] = slider;
        customKnobLightTrough[slotIndex] = customLightTrough;
        labels[slotIndex].setText(label, juce::dontSendNotification);
        labels[slotIndex].setVisible(false);
        if (slider) {
            slider->setLabel(label);
            slider->setAccentColour(customAccent.value_or(accent));
            slider->setLightTrough(customLightTrough.value_or(knobsLightTrough));
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

    auto compColour = panelTintBaseColour.isTransparent() ? accent.withRotatedHue(0.5f) : panelTintBaseColour;
    auto panelBg = juce::Colour(0xff13161f).interpolatedWith(compColour, 0.16f);
    auto panelBorder = juce::Colour(0xff222736).interpolatedWith(compColour, 0.20f);

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

juce::String formatPercent200(double val) {
    return juce::String(static_cast<int>(std::round(val * 200.0))) + "%";
}
double parsePercent200(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 100.0) / 200.0, 0.0, 1.0);
}

juce::String formatCrossfade(double val) {
    int p = static_cast<int>(std::round((val - 0.5) * 200.0));
    if (p == 0) return "0% (Both)";
    if (p < 0)  return juce::String(p) + "% (Noise)";
    return "+" + juce::String(p) + "% (FM)";
}
double parseCrossfade(const juce::String& text) {
    double p = parseNumberSafe(text, 0.0);
    return std::clamp((p / 200.0) + 0.5, 0.0, 1.0);
}

// --- CENTRALIZED TOOLTIP HELPERS ---

namespace TooltipHelper {

juce::String makeKnobTooltip(const juce::String& title,
                             const juce::String& description,
                             const juce::String& defaultAndUnits,
                             bool isBipolar)
{
    juce::String tip = title;
    if (isBipolar) tip += " [Bipolar +/-]";
    if (description.isNotEmpty()) tip += ": " + description;
    if (defaultAndUnits.isNotEmpty()) {
        tip += "\nDefault: " + defaultAndUnits + " | Double-click to reset";
    } else {
        tip += "\nDouble-click to reset";
    }
    return tip;
}

juce::String makeKnobTooltipFromParam(juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& paramId,
                                      const juce::String& fallbackDesc,
                                      bool fallbackBipolar)
{
    auto* param = apvts.getParameter(paramId);
    juce::String desc = fallbackDesc;
    bool isBipolar = fallbackBipolar;

    if (auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId)) {
        if (def->description.isNotEmpty()) desc = def->description;
        isBipolar = def->isBipolar;
    }

    if (param != nullptr) {
        juce::String name = param->getName(64);
        float defVal = param->getDefaultValue();
        juce::String defText = param->getText(defVal, 32);
        juce::String label = param->getLabel();
        juce::String defAndUnits = defText;
        if (label.isNotEmpty() && !defText.endsWithIgnoreCase(label)) {
            defAndUnits += " " + label;
        }
        return makeKnobTooltip(name, desc, defAndUnits, isBipolar);
    }
    return makeKnobTooltip(paramId, desc, "", isBipolar);
}

juce::StringArray getLedSelectorItemTooltips(const juce::String& paramId)
{
    auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
    if (def && !def->choiceTooltips.isEmpty()) {
        return def->choiceTooltips;
    }
    return {};
}

juce::String getFxAlgorithmTooltip(int fxIndex) {
    switch (fxIndex) {
        case 1:  return "BELL EQ: Parametric peaking/notching equalizer with variable frequency, Q bandwidth, +/-12 dB gain, and DJ tilt filter.";
        case 2:  return "CHORUS: Multi-voice modulated delay lines creating stereo shimmer, depth, and spatial width.";
        case 3:  return "COMB FILTER: Tuned resonant delay feedback loop with high dampening, cutoff frequency, and bipolar feedback.";
        case 4:  return "DRIVE: Nonlinear analog saturation with harmonic drive gain, DC bias asymmetry, pre-filtering, and output limiter.";
        case 5:  return "FX FILTER: Multi-mode resonant state-variable filter with selectable type, slope (6-36 dB/oct), cutoff, and resonance.";
        case 6:  return "FLANGER: Short modulated delay line with high regenerative feedback, creating dynamic sweeping comb filter notches.";
        case 7:  return "FREQ SHIFTER: Frequency shifter using Hilbert transform quadrature processing with bipolar shift frequency, scaling range, and stereo width.";
        case 8:  return "GRIT FX: Lo-fi digital degrader with variable bit-depth reduction (1-16 bits), sample-rate crushing, and low/high EQ tone shaping.";
        case 9:  return "PHASE SMEAR: Cascade of 2nd or 4th order all-pass dispersion filters for laser zaps, transient dispersion, and resonant smearing.";
        case 10: return "PHASER: Multi-stage all-pass phasing network with LFO modulation rate, sweep depth, regenerative feedback, and wet/dry mix.";
        case 11: return "RINGMOD FX: Ring modulator multiplying audio by an internal variable-waveform oscillator (sine to square), with LFO rate, amount, and stereo width.";
        case 12: return "TEMPO DELAY: Tempo-synchronized stereo delay with musical beat divisions (1/32 to 1/2), feedback regeneration, low-pass tone damping, and mix.";
        case 13: return "WAVE FOLDER: West-Coast style harmonic wavefolding distortion with fold amount, symmetry bias, pre-tilt filtering, and output limiter.";
        default: return "BYPASS: FX processing bypassed, audio passes through clean.";
    }
}

juce::String getFxKnobTooltip(int fxIndex, int knobIndex) {
    auto* alg = RlyehSound::ParameterManager::getInstance().getFxAlgorithmDef(fxIndex);
    if (alg && knobIndex >= 0 && knobIndex < alg->knobParams.size()) {
        auto paramId = alg->knobParams[knobIndex];
        auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
        if (def) {
            return makeKnobTooltip(def->name, def->description, def->defaultLabel, def->isBipolar);
        }
    }
    return makeKnobTooltip("Parameter " + juce::String(knobIndex + 1), "Slot parameter for the active effect");
}

} // namespace TooltipHelper


// ==============================================================================
// AdvancedColorPickerComponent
// ==============================================================================

class AdvancedColorPickerComponent::PaletteSwatch : public juce::Component {
public:
    juce::Colour color;
    std::function<void(juce::Colour)> onSelect;
    std::function<void(PaletteSwatch*)> onSave;
    
    void paint(juce::Graphics& g) override {
        g.fillAll(color);
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRect(getLocalBounds(), 1);
    }
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isRightButtonDown()) {
            if (onSave) onSave(this);
        } else {
            if (onSelect) onSelect(color);
        }
    }
};

AdvancedColorPickerComponent::AdvancedColorPickerComponent(juce::Colour initialColor, std::function<void(juce::Colour)> onColorChangedFunc)
    : onColorChanged(onColorChangedFunc) {
    
    auto setupSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text, double maxVal, const juce::String& suffix) {
        addAndMakeVisible(s);
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        s.setRange(0.0, maxVal, 1.0);
        s.setTextValueSuffix(suffix);
        s.addListener(this);
        
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.attachToComponent(&s, true);
    };

    setupSlider(rSlider, rLabel, "R", 255.0, "");
    setupSlider(gSlider, gLabel, "G", 255.0, "");
    setupSlider(bSlider, bLabel, "B", 255.0, "");
    setupSlider(aSlider1, aLabel1, "A", 255.0, "");

    setupSlider(hSlider, hLabel, "H", 360.0, " deg");
    setupSlider(sSlider, sLabel, "S", 100.0, "%");
    setupSlider(vSlider, vLabel, "V", 100.0, "%");
    setupSlider(aSlider2, aLabel2, "A", 100.0, "%");
    
    addAndMakeVisible(headerRgba);
    headerRgba.setText("R G B A", juce::dontSendNotification);
    addAndMakeVisible(headerHsva);
    headerHsva.setText("H S V A", juce::dontSendNotification);

    addAndMakeVisible(hexInput);
    hexInput.addListener(this);
    addAndMakeVisible(hexLabel);
    hexLabel.setText("#", juce::dontSendNotification);
    hexLabel.attachToComponent(&hexInput, true);

    addAndMakeVisible(resetButton);

    resetButton.onClick = [this]() {
        updateFromColor(originalColor, true, true, true);
    };

    for (int i = 0; i < 16; ++i) {
        customColors[i] = juce::Colour(0xff303030);
    }
    loadPreferences();

    for (int i = 0; i < 16; ++i) {
        auto* swatch = new PaletteSwatch();
        swatch->color = customColors[i];
        swatch->onSelect = [this](juce::Colour c) { updateFromColor(c, true, true, true); };
        swatch->onSave = [this, i](PaletteSwatch* s) { 
            s->color = currentColor; 
            customColors[i] = currentColor;
            s->repaint();
            savePreferences();
        };
        swatches.add(swatch);
        addAndMakeVisible(swatch);
    }

    originalColor = initialColor;
    updateFromColor(initialColor, false, true, true);

    setSize(520, 360);
}

AdvancedColorPickerComponent::~AdvancedColorPickerComponent() {}

void AdvancedColorPickerComponent::loadPreferences() {
    juce::File prefsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("RlyehSound").getChildFile("TheKlangFarmer").getChildFile("preferences.json");
    if (prefsFile.existsAsFile()) {
        auto parsed = juce::JSON::parse(prefsFile);
        if (parsed.isObject()) {
            auto arr = parsed.getProperty("custom_palette", juce::var());
            if (arr.isArray()) {
                auto* array = arr.getArray();
                for (int i = 0; i < juce::jmin(16, array->size()); ++i) {
                    customColors[i] = juce::Colour::fromString(array->getReference(i).toString());
                }
            }
        }
    }
}

void AdvancedColorPickerComponent::savePreferences() {
    juce::File prefsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("RlyehSound").getChildFile("TheKlangFarmer").getChildFile("preferences.json");
    prefsFile.getParentDirectory().createDirectory();
    
    juce::var prefsObj(new juce::DynamicObject());
    if (prefsFile.existsAsFile()) {
        auto existing = juce::JSON::parse(prefsFile);
        if (existing.isObject()) prefsObj = existing;
    }
    
    juce::Array<juce::var> arr;
    for (int i = 0; i < 16; ++i) {
        arr.add(customColors[i].toDisplayString(true));
    }
    prefsObj.getDynamicObject()->setProperty("custom_palette", arr);
    
    prefsFile.replaceWithText(juce::JSON::toString(prefsObj));
}

juce::Rectangle<float> AdvancedColorPickerComponent::getRingBounds() const {
    return juce::Rectangle<float>(20, 20, 240, 240);
}

juce::Rectangle<float> AdvancedColorPickerComponent::getInnerSquareBounds() const {
    auto b = getRingBounds();
    return juce::Rectangle<float>(b.getCentreX() - 55, b.getCentreY() - 55, 110, 110);
}

void AdvancedColorPickerComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff2b2d31));

    auto ringBounds = getRingBounds();
    float centerX = ringBounds.getCentreX();
    float centerY = ringBounds.getCentreY();
    float outerRadius = 120.0f;
    float innerRadius = 80.0f;

    for (float angle = 0.0f; angle < juce::MathConstants<float>::twoPi; angle += 0.02f) {
        float hue = angle / juce::MathConstants<float>::twoPi;
        g.setColour(juce::Colour::fromHSV(hue, 1.0f, 1.0f, 1.0f));
        juce::Path p;
        p.addCentredArc(centerX, centerY, (outerRadius+innerRadius)*0.5f, (outerRadius+innerRadius)*0.5f, 
                        0.0f, angle, angle + 0.03f, true);
        g.strokePath(p, juce::PathStrokeType(outerRadius - innerRadius));
    }

    float cursorAngle = currentHue * juce::MathConstants<float>::twoPi;
    float cursorRadius = (outerRadius + innerRadius) * 0.5f;
    float cx = centerX + cursorRadius * std::sin(cursorAngle);
    float cy = centerY - cursorRadius * std::cos(cursorAngle);
    g.setColour(juce::Colours::white);
    g.drawEllipse(cx - 10, cy - 10, 20, 20, 2.5f);

    auto innerSquare = getInnerSquareBounds();
    g.setColour(currentColor);
    g.fillRoundedRectangle(innerSquare, 10.0f);

    g.setColour(currentColor.contrasting());
    g.setFont(16.0f);
    g.drawText(currentColor.toDisplayString(true).toUpperCase(), 
               innerSquare.withTrimmedTop(80).toNearestInt(), juce::Justification::centred, false);
}

void AdvancedColorPickerComponent::resized() {
    int sliderX = 320;
    int sliderW = 180;
    int y = 20;

    headerRgba.setBounds(sliderX, y, sliderW, 20); y += 24;
    rSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    gSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    bSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    aSlider1.setBounds(sliderX, y, sliderW, 20); y += 30;

    headerHsva.setBounds(sliderX, y, sliderW, 20); y += 24;
    hSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    sSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    vSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    aSlider2.setBounds(sliderX, y, sliderW, 20); y += 30;

    hexInput.setBounds(sliderX, y, 110, 24);
    resetButton.setBounds(sliderX + 120, y, 60, 24);

    
    // 2x8 palette grid
    int px = 20;
    int py = 290;
    int pw = 25;
    for (int i = 0; i < 16; ++i) {
        swatches[i]->setBounds(px + (i % 8) * (pw + 5), py + (i / 8) * (pw + 5), pw, pw);
    }
}

void AdvancedColorPickerComponent::mouseDown(const juce::MouseEvent& e) {
    mouseDrag(e);
}

void AdvancedColorPickerComponent::mouseDrag(const juce::MouseEvent& e) {
    auto b = getRingBounds();
    float dx = e.x - b.getCentreX();
    float dy = e.y - b.getCentreY();
    float distance = std::sqrt(dx*dx + dy*dy);
    
    if (distance > 10.0f) {
        float angle = std::atan2(dx, -dy);
        if (angle < 0) angle += juce::MathConstants<float>::twoPi;
        currentHue = angle / juce::MathConstants<float>::twoPi;
        updateFromHSVA(false, true, true); // Do NOT notify on every pixel
    }
}

void AdvancedColorPickerComponent::mouseUp(const juce::MouseEvent& e) {
    if (onColorChanged) onColorChanged(currentColor); // Notify only when released
}



void AdvancedColorPickerComponent::textEditorTextChanged(juce::TextEditor& editor) {
    if (isUpdating) return;
    if (&editor == &hexInput) {
        juce::String hex = hexInput.getText().toUpperCase();
        if (hex.startsWith("0X") && hex.length() == 10) {
            updateFromColor(juce::Colour::fromString(hex), true, true, false);
        }
    }
}

void AdvancedColorPickerComponent::textEditorReturnKeyPressed(juce::TextEditor& editor) {
    textEditorTextChanged(editor);
}

void AdvancedColorPickerComponent::updateFromColor(juce::Colour newColor, bool notify, bool updateSliders, bool updateHex) {
    currentColor = newColor;
    currentColor.getHSB(currentHue, currentSat, currentVal);
    currentAlpha = currentColor.getFloatAlpha();
    
    if (updateSliders) updateSlidersFromColor();
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}

void AdvancedColorPickerComponent::updateFromHSVA(bool notify, bool updateRGB, bool updateHex) {
    currentColor = juce::Colour(currentHue, currentSat, currentVal, currentAlpha);
    
    isUpdating = true;
    if (updateRGB) {
        rSlider.setValue(currentColor.getRed(), juce::dontSendNotification);
        gSlider.setValue(currentColor.getGreen(), juce::dontSendNotification);
        bSlider.setValue(currentColor.getBlue(), juce::dontSendNotification);
        aSlider1.setValue(currentColor.getAlpha(), juce::dontSendNotification);
    }
    hSlider.setValue(currentHue * 360.0, juce::dontSendNotification);
    sSlider.setValue(currentSat * 100.0, juce::dontSendNotification);
    vSlider.setValue(currentVal * 100.0, juce::dontSendNotification);
    aSlider2.setValue(currentAlpha * 100.0, juce::dontSendNotification);
    isUpdating = false;
    
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}

void AdvancedColorPickerComponent::updateFromRGBA(bool notify, bool updateHSV, bool updateHex) {
    currentColor = juce::Colour(
        (juce::uint8)rSlider.getValue(),
        (juce::uint8)gSlider.getValue(),
        (juce::uint8)bSlider.getValue(),
        (juce::uint8)aSlider1.getValue()
    );
    currentColor.getHSB(currentHue, currentSat, currentVal);
    currentAlpha = currentColor.getFloatAlpha();
    
    isUpdating = true;
    if (updateHSV) {
        hSlider.setValue(currentHue * 360.0, juce::dontSendNotification);
        sSlider.setValue(currentSat * 100.0, juce::dontSendNotification);
        vSlider.setValue(currentVal * 100.0, juce::dontSendNotification);
        aSlider2.setValue(currentAlpha * 100.0, juce::dontSendNotification);
    }
    isUpdating = false;
    
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}

void AdvancedColorPickerComponent::updateHexFromColor() {
    isUpdating = true;
    hexInput.setText(currentColor.toDisplayString(true).toUpperCase(), juce::dontSendNotification);
    isUpdating = false;
}

void AdvancedColorPickerComponent::updateSlidersFromColor() {
    isUpdating = true;
    rSlider.setValue(currentColor.getRed(), juce::dontSendNotification);
    gSlider.setValue(currentColor.getGreen(), juce::dontSendNotification);
    bSlider.setValue(currentColor.getBlue(), juce::dontSendNotification);
    aSlider1.setValue(currentColor.getAlpha(), juce::dontSendNotification);
    
    hSlider.setValue(currentHue * 360.0, juce::dontSendNotification);
    sSlider.setValue(currentSat * 100.0, juce::dontSendNotification);
    vSlider.setValue(currentVal * 100.0, juce::dontSendNotification);
    aSlider2.setValue(currentAlpha * 100.0, juce::dontSendNotification);
    isUpdating = false;
}

void AdvancedColorPickerComponent::sliderValueChanged(juce::Slider* slider) {
    if (isUpdating) return;
    
    if (slider == &rSlider || slider == &gSlider || slider == &bSlider || slider == &aSlider1) {
        updateFromRGBA(false, true, true); // Do NOT notify on every pixel
    } else if (slider == &hSlider || slider == &sSlider || slider == &vSlider || slider == &aSlider2) {
        currentHue = hSlider.getValue() / 360.0f;
        currentSat = sSlider.getValue() / 100.0f;
        currentVal = vSlider.getValue() / 100.0f;
        currentAlpha = aSlider2.getValue() / 100.0f;
        updateFromHSVA(false, true, true); // Do NOT notify on every pixel
    }
}

void AdvancedColorPickerComponent::sliderDragEnded(juce::Slider* slider) {
    if (onColorChanged) onColorChanged(currentColor); // Notify only when released
}
