#pragma once
#include <vector>
#include <JuceHeader.h>

struct DiffLine {
    juce::String text;
    int type; // 0 = equal, 1 = insert, -1 = delete
};

inline std::vector<DiffLine> computeDiff(const juce::StringArray& oldLines, const juce::StringArray& newLines) {
    std::vector<DiffLine> result;
    int n = oldLines.size();
    int m = newLines.size();
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
    
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (oldLines[i - 1] == newLines[j - 1]) dp[i][j] = dp[i - 1][j - 1] + 1;
            else dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
        }
    }
    
    int i = n, j = m;
    std::vector<DiffLine> revResult;
    while (i > 0 && j > 0) {
        if (oldLines[i - 1] == newLines[j - 1]) {
            revResult.push_back(DiffLine{oldLines[i - 1], 0});
            i--; j--;
        } else if (dp[i - 1][j] > dp[i][j - 1]) {
            revResult.push_back(DiffLine{oldLines[i - 1], -1});
            i--;
        } else {
            revResult.push_back(DiffLine{newLines[j - 1], 1});
            j--;
        }
    }
    while (i > 0) { revResult.push_back(DiffLine{oldLines[i - 1], -1}); i--; }
    while (j > 0) { revResult.push_back(DiffLine{newLines[j - 1], 1}); j--; }
    
    for (int k = revResult.size() - 1; k >= 0; --k) result.push_back(revResult[k]);
    return result;
}

class DiffListBoxModel : public juce::ListBoxModel {
public:
    std::vector<DiffLine> lines;
    int getNumRows() override { return lines.size(); }
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (row < 0 || row >= lines.size()) return;
        auto& dl = lines[row];
        if (dl.type == 1) g.fillAll(juce::Colour(0xff2ea043).withAlpha(0.3f)); // Green for added
        else if (dl.type == -1) g.fillAll(juce::Colour(0xfff85149).withAlpha(0.3f)); // Red for removed
        
        g.setColour(juce::Colours::white);
        g.drawText(dl.text, 4, 0, width - 8, height, juce::Justification::centredLeft, true);
    }
};

class DiffViewerComponent : public juce::Component {
public:
    DiffViewerComponent(const juce::String& oldStr, const juce::String& newStr) {
        juce::StringArray oldLines, newLines;
        oldLines.addLines(oldStr);
        newLines.addLines(newStr);
        model.lines = computeDiff(oldLines, newLines);
        
        listBox.setModel(&model);
        listBox.setRowHeight(20);
        addAndMakeVisible(listBox);
        setSize(800, 600);
    }
    
    void resized() override {
        listBox.setBounds(getLocalBounds());
    }
private:
    DiffListBoxModel model;
    juce::ListBox listBox;
};
