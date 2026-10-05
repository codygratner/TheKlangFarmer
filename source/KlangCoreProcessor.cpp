#include "KlangCoreProcessor.h"

KlangCoreProcessor::KlangCoreProcessor(const BusesProperties& ioLayouts, const juce::String& apvtsName, juce::AudioProcessorValueTreeState::ParameterLayout layout)
    : AudioProcessor(ioLayouts), apvts(*this, nullptr, apvtsName, std::move(layout))
{
}

void KlangCoreProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void KlangCoreProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr) {
        if (xmlState->hasTagName(apvts.state.getType())) {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

