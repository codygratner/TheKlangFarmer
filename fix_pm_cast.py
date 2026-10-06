with open('source/ParameterManager.cpp', 'r') as f:
    content = f.read()

content = content.replace(
    'poi.value = poiObj->hasProperty("value") ? static_cast<float>(poiObj->getProperty("value")) : 0.0f;',
    'poi.value = poiObj->hasProperty("value") ? static_cast<float>(double(poiObj->getProperty("value"))) : 0.0f;'
)

with open('source/ParameterManager.cpp', 'w') as f:
    f.write(content)
