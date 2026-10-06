#include <iostream>
#include "ParameterManager.h"

int main() {
    std::cout << "--- BENCHMARK START ---" << std::endl;
    auto& pm = RlyehSound::ParameterManager::getInstance();
    auto* def = pm.getControlDef("carrier1_shape");
    if (def) {
        std::cout << "FOUND carrier1_shape! Snap points: " << def->snapPoints.size() << std::endl;
        for(const auto& p : def->snapPoints) {
            std::cout << "  - " << p.value << " : " << p.label.toStdString() << std::endl;
        }
    } else {
        std::cout << "NOT FOUND" << std::endl;
    }
    return 0;
}
