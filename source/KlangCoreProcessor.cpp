#include "KlangCoreProcessor.h"

KlangCoreProcessor::KlangCoreProcessor(const BusesProperties& ioLayouts, const juce::String& apvtsName, juce::AudioProcessorValueTreeState::ParameterLayout layout)
    : AudioProcessor(ioLayouts), apvts(*this, &undoManager, apvtsName, std::move(layout))
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

void KlangCoreProcessor::saveToBufferA() {
    getStateInformation(stateBufferA);
    hasBufferA = true;
}

void KlangCoreProcessor::saveToBufferB() {
    getStateInformation(stateBufferB);
    hasBufferB = true;
}

void KlangCoreProcessor::copyAToB() {
    if (!hasBufferA) saveToBufferA();
    stateBufferB = stateBufferA;
    hasBufferB = true;
}

void KlangCoreProcessor::copyBToA() {
    if (!hasBufferB) saveToBufferB();
    stateBufferA = stateBufferB;
    hasBufferA = true;
}

bool KlangCoreProcessor::toggleAB() {
    if (isViewingBufferB) {
        saveToBufferB();
        if (hasBufferA) {
            setStateInformation(stateBufferA.getData(), static_cast<int>(stateBufferA.getSize()));
        }
        isViewingBufferB = false;
    } else {
        saveToBufferA();
        if (!hasBufferB) {
            copyAToB();
        } else {
            setStateInformation(stateBufferB.getData(), static_cast<int>(stateBufferB.getSize()));
        }
        isViewingBufferB = true;
    }
    return isViewingBufferB;
}

