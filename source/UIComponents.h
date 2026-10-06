#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <cmath>
#include <cctype>

#include "ModularBlocks.h"

// Modulation Info structs
struct ModSourceDetail {
    juce::String name;
    juce::String depthText;
};

struct ParamModulationInfo {
    bool isModulated = false;
    float rangeMinNorm = 0.0f;
    float rangeMaxNorm = 0.0f;
    float currentNorm = 0.0f;
    bool showNeedle = true;
    juce::String rangeText;
    juce::String liveValueText;
    std::vector<ModSourceDetail> sources;
};

// --- SAFE PARSING & FORMATTING HELPERS ---

double parseNumberSafe(const juce::String& text, double fallback);
juce::String getMidiNoteName(int noteNumber);
juce::String formatMidiNote(double val);
double parseMidiNote(const juce::String& text);
juce::String formatSemi(double val);
double parseSemi(const juce::String& text);
juce::String formatSemi24(double val);
double parseSemi24(const juce::String& text);
juce::String formatCarrierFreqHz(double val);
double parseCarrierFreqHz(const juce::String& text);
juce::String formatNoteDetail(double val);
double parseNoteDetail(const juce::String& text);
juce::String formatRatio(double val);
double parseRatio(const juce::String& text);
juce::String formatPercent(double val);
double parsePercent(const juce::String& text);
juce::String formatPercent200(double val);
double parsePercent200(const juce::String& text);
juce::String formatBipolarPercent(double val);
double parseBipolarPercent(const juce::String& text);
juce::String formatCrossfade(double val);
double parseCrossfade(const juce::String& text);
juce::String formatTimeMs(double val);
double parseTimeMs(const juce::String& text);
juce::String formatNoiseTimeMs(double val);
double parseNoiseTimeMs(const juce::String& text);
juce::String formatFreqHz(double val);
double parseFreqHz(const juce::String& text);
juce::String formatEqFreqHz(double val);
double parseEqFreqHz(const juce::String& text);
juce::String formatDb(double val);
double parseDb(const juce::String& text);
juce::String formatAmpDriveDb(double val);
double parseAmpDriveDb(const juce::String& text);
juce::String formatVelocityFloor(double val);
double parseVelocityFloor(const juce::String& text);
juce::String formatBipolarDb(double val);
double parseBipolarDb(const juce::String& text);
juce::String formatWaveshape(double val);
double parseWaveshape(const juce::String& text);

juce::String formatWetDry(double val);
double parseWetDry(const juce::String& text);
juce::String formatMixerLevel(double val);
double parseMixerLevel(const juce::String& text);
juce::String formatOctaves(double val);
double parseOctaves(const juce::String& text);
juce::String formatFilterOctaves(double val);
double parseFilterOctaves(const juce::String& text);
juce::String formatBits(double val);
double parseBits(const juce::String& text);
juce::String formatWavefolds(double val);
double parseWavefolds(const juce::String& text);
juce::String formatStages(double val);
double parseStages(const juce::String& text);
juce::String formatClaps(double val);
double parseClaps(const juce::String& text);
juce::String formatClapSpeed(double val);
double parseClapSpeed(const juce::String& text);
juce::String formatLimiterGain(double val);
double parseLimiterGain(const juce::String& text);
juce::String formatLimiterThresh(double val);
double parseLimiterThresh(const juce::String& text);
juce::String formatLimiterRelease(double val);
double parseLimiterRelease(const juce::String& text);
juce::String formatSlop(double val);
double parseSlop(const juce::String& text);
juce::String formatVelocitySlope(double val);
double parseVelocitySlope(const juce::String& text);
juce::String formatSlope(double val);
double parseSlope(const juce::String& text);
juce::String formatEqWidthOct(double val);
double parseEqWidthOct(const juce::String& text);
juce::String formatRangeHz(double val);
double parseRangeHz(const juce::String& text);
juce::String formatChorusRate(double val);
double parseChorusRate(const juce::String& text);
juce::String formatPhaserRate(double val);
double parsePhaserRate(const juce::String& text);
juce::String formatFlangerRate(double val);
double parseFlangerRate(const juce::String& text);
juce::String formatDelayDiv(double val);
double parseDelayDiv(const juce::String& text);
juce::String formatDelayTone(double val);
double parseDelayTone(const juce::String& text);

// Sleek LookAndFeel for rotary knobs

class AdvancedColorPickerComponent : public juce::Component, public juce::Slider::Listener, public juce::TextEditor::Listener {
public:
    AdvancedColorPickerComponent(juce::Colour initialColor, std::function<void(juce::Colour)> onColorChangedFunc);
    ~AdvancedColorPickerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void sliderValueChanged(juce::Slider* slider) override;
    void sliderDragEnded(juce::Slider* slider) override;
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

private:
    juce::Colour originalColor;
    juce::Colour currentColor;
    float currentHue, currentSat, currentVal, currentAlpha;
    bool isUpdating = false;
    bool isDraggingRing = false;

    std::function<void(juce::Colour)> onColorChanged;

    void updateFromColor(juce::Colour newColor, bool notify, bool updateSliders, bool updateHex);
    void updateFromHSVA(bool notify, bool updateRGB, bool updateHex);
    void updateFromRGBA(bool notify, bool updateHSV, bool updateHex);
    void updateHexFromColor();
    void updateSlidersFromColor();
    
    void loadPreferences();
    void savePreferences();

    juce::Rectangle<float> getRingBounds() const;
    juce::Rectangle<float> getInnerSquareBounds() const;

    juce::Slider rSlider, gSlider, bSlider, aSlider1;
    juce::Slider hSlider, sSlider, vSlider, aSlider2;
    juce::Label rLabel, gLabel, bLabel, aLabel1;
    juce::Label hLabel, sLabel, vLabel, aLabel2;
    juce::Label headerRgba, headerHsva, hexLabel;
    juce::TextEditor hexInput;
    juce::TextButton resetButton { "Reset" };

    class PaletteSwatch;
    std::array<juce::Colour, 16> customColors;
    juce::OwnedArray<PaletteSwatch> swatches;
};

class RotaryKnobLookAndFeel : public juce::LookAndFeel_V4 {
public:
    RotaryKnobLookAndFeel();
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
    juce::Label* createSliderTextBox(juce::Slider& slider) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;

    juce::Rectangle<int> getTooltipBounds(const juce::String& tipText,
                                          juce::Point<int> screenPos,
                                          juce::Rectangle<int> parentArea) override;
    void drawTooltip(juce::Graphics& g,
                     const juce::String& text,
                     int width,
                     int height) override;
};

// Mini Oscilloscope or Frequency-Gain Plot widget
class MiniOscilloscopeComponent : public juce::Component {
public:
    enum class PlotMode {
        Oscilloscope,
        FilterXY,
        EqXY
    };

    MiniOscilloscopeComponent(juce::Colour traceColour);
    void updateData(const float* data, int numPoints);
    void setPlotMode(PlotMode mode);
    void updateFilterParams(int type, int slope, float cutoffHz, float resonance);
    void updateEqParams(float freqHz, float widthOct, float gainDb, float djFilter);
    void paint(juce::Graphics& g) override;

private:
    juce::Colour traceCol;
    PlotMode plotMode = PlotMode::Oscilloscope;
    std::vector<float> points;

    int filterType = 0;
    int filterSlope = 1;
    float filterCutoff = 24000.0f;
    float filterResonance = 0.0f;

    float eqFreq = 24000.0f;
    float eqWidth = 0.1f;
    float eqGain = 0.0f;
    float eqDJ = 0.5f;
};

// Arcade HP Meter Slider with dual-axis dragging, embedded label, static right value, and waveform/slope diagram support
class RotaryKnobSlider : public juce::Slider {
public:
    enum class DiagramType {
        None,
        Waveform,
        EnvelopeSlope,
        VelocitySlope,
        FilterSlope
    };

    RotaryKnobSlider();
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    void openHoveringEditor();

    void setLabel(const juce::String& l) { label = l; repaint(); }
    juce::String getLabel() const { return label; }

    void setAccentColour(juce::Colour c) { accentColour = c; repaint(); }
    juce::Colour getAccentColour() const { return accentColour; }

    bool isBipolar = false;
    void setBipolar(bool bipolar) { isBipolar = bipolar; repaint(); }

    bool isLightTrough = false;
    void setLightTrough(bool light) { isLightTrough = light; repaint(); }

    DiagramType diagramType = DiagramType::None;

    std::function<double()> getDefaultValue;
    std::function<juce::String(double)> customFormatText;
    std::function<double(const juce::String&)> customParseText;

    juce::String getTextFromValue(double val) override;
    double getValueFromText(const juce::String& text) override;

    struct ModulationVisual {
        bool isModulated = false;
        float rangeMinNorm = 0.0f;
        float rangeMaxNorm = 0.0f;
        float currentNorm = 0.0f;
        bool showNeedle = true;
    };
    ModulationVisual modulation;

    void setModulation(const ModulationVisual& mod) {
        if (modulation.isModulated != mod.isModulated ||
            modulation.showNeedle != mod.showNeedle ||
            std::abs(modulation.rangeMinNorm - mod.rangeMinNorm) > 0.001f ||
            std::abs(modulation.rangeMaxNorm - mod.rangeMaxNorm) > 0.001f ||
            std::abs(modulation.currentNorm - mod.currentNorm) > 0.001f) {
            modulation = mod;
            repaint();
        }
    }

    void setParamId(const juce::String& id) { paramId = id; }
    const juce::String& getParamId() const { return paramId; }

    std::function<ParamModulationInfo(const juce::String&)> getModInfoFunc;

    void paint(juce::Graphics& g) override;

private:
    juce::String label;
    juce::String paramId;
    juce::Colour accentColour { 0xff00d2ff };
    juce::Point<int> dragStartPos;
    double dragStartVal = 0.0;

    void drawDiagram(juce::Graphics& g, juce::Rectangle<float> area);
};

// Interactive callout popup on right-click allowing slider value editing and displaying active modulation info
class SliderCalloutComponent : public juce::Component, public juce::Timer {
public:
    SliderCalloutComponent(RotaryKnobSlider& ownerSlider,
                           const juce::String& pId,
                           std::function<ParamModulationInfo(const juce::String&)> modGetter);
    ~SliderCalloutComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void visibilityChanged() override;

private:
    RotaryKnobSlider& slider;
    juce::String paramId;
    std::function<ParamModulationInfo(const juce::String&)> getModInfo;
    ParamModulationInfo cachedInfo;
    juce::TextEditor editor;
    juce::String lastLiveText;
    juce::OwnedArray<juce::TextButton> presetButtons;
};

// Custom diagram-rendering label used as slider text box
class DiagramSliderLabel : public juce::Label, public juce::Slider::Listener {
public:
    explicit DiagramSliderLabel(RotaryKnobSlider& s);
    ~DiagramSliderLabel() override;
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void sliderValueChanged(juce::Slider*) override { repaint(); }

private:
    RotaryKnobSlider& slider;
};

// Sleek hardware-style LED list selector
class LedSelectorComponent : public juce::Component,
                             public juce::SettableTooltipClient {
public:
    struct ItemStyle {
        juce::Colour primaryAccent;
        std::optional<juce::Colour> secondaryAccent = std::nullopt;
        std::optional<juce::Colour> textColour      = std::nullopt;
    };

    explicit LedSelectorComponent(juce::Colour activeAccent);

    void setItems(const juce::StringArray& newItems, int numColumns = 2);
    void setSelectedIndex(int newIndex, juce::NotificationType notification = juce::sendNotificationAsync);
    int getSelectedIndex() const { return selectedIndex; }
    int getNumItems() const { return items.size(); }
    void setParamId(const juce::String& id) { paramId = id; }
    const juce::String& getParamId() const { return paramId; }
    void setAccent(juce::Colour col) { accent = col; repaint(); }

    void setItemStyle(int index, const ItemStyle& style);
    void clearItemStyles();

    void setItemTooltips(const juce::StringArray& tooltips);
    void setItemTooltip(int index, const juce::String& tooltip);
    juce::String getTooltip() override;

    std::function<void(int)> onChange;

    void paint(juce::Graphics& g) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    juce::StringArray items;
    juce::StringArray itemTooltips;
    std::vector<std::optional<ItemStyle>> itemStyles;
    int selectedIndex = 0;
    int hoveredIndex = -1;
    int columns = 2;
    juce::String paramId;
    juce::Colour accent;

    int getItemIndexAt(juce::Point<int> pos) const;
    juce::Rectangle<int> getItemBounds(int index) const;
};

// Card component representing one modular block in the rack
class ModuleCardComponent : public juce::Component,
                            public juce::SettableTooltipClient {
public:
    enum class PanelStyle {
        StandardDark,
        DoepferSilver
    };

    ModuleCardComponent(const juce::String& title, juce::Colour accentColour, PanelStyle style = PanelStyle::StandardDark);
    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPanelStyle(PanelStyle style);
    PanelStyle getPanelStyle() const { return panelStyle; }
    void setPanelTintBaseColour(juce::Colour c) { panelTintBaseColour = c; repaint(); }
    void setAccentColour(juce::Colour c) { accent = c; repaint(); }
    juce::Colour getAccentColour() const { return accent; }

    void updateScope(const float* data, int numSamples);
    void setPlotMode(MiniOscilloscopeComponent::PlotMode mode);
    void updateFilterParams(int type, int slope, float cutoffHz, float resonance);
    void updateEqParams(float freqHz, float widthOct, float gainDb, float djFilter);
    void setLedSelector(LedSelectorComponent* selector);
    void setSecondLedSelector(LedSelectorComponent* selector);
    void setSelector(juce::ComboBox* box);
    void setSelectorAtBottom(bool atBottom);
    void setKnobsLightTrough(bool lightTrough);
    void setKnob(int slotIndex, const juce::String& label, RotaryKnobSlider* slider,
                 std::optional<juce::Colour> customAccent = std::nullopt,
                 std::optional<bool> customLightTrough = std::nullopt);
    void setKnobLabel(int slotIndex, const juce::String& label);
    void setNumActiveKnobs(int count) { numActiveKnobs = count; }

    juce::String getTitle() const { return moduleTitle; }

    void mouseDown(const juce::MouseEvent& e) override;
    std::function<void()> onCardClicked;

private:
    juce::String moduleTitle;
    juce::Colour accent;
    juce::Colour panelTintBaseColour;
    PanelStyle panelStyle = PanelStyle::StandardDark;
    MiniOscilloscopeComponent oscilloscope;

    LedSelectorComponent* ledSelector = nullptr;
    LedSelectorComponent* secondLedSelector = nullptr;
    juce::ComboBox* selectorBox = nullptr;
    bool selectorAtBottom = false;
    bool knobsLightTrough = false;
    std::optional<bool> customKnobLightTrough[4];
    juce::Label labels[4];
    RotaryKnobSlider* knobs[4] = {};
    int numActiveKnobs = 4;
};

// Centralized Tooltip Formatting & Metadata Helpers
namespace TooltipHelper {
    juce::String makeKnobTooltip(const juce::String& title,
                                 const juce::String& description,
                                 const juce::String& defaultAndUnits = {},
                                 bool isBipolar = false);

    juce::String makeKnobTooltipFromParam(juce::AudioProcessorValueTreeState& apvts,
                                          const juce::String& paramId,
                                          const juce::String& description,
                                          bool isBipolar = false);

    juce::StringArray getLedSelectorItemTooltips(const juce::String& paramId);
    juce::String getFxAlgorithmTooltip(int fxIndex);
    juce::String getFxKnobTooltip(int fxIndex, int knobIndex);
}