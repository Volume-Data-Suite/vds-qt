#pragma once

#include "light_source.h"

namespace VDS {

LightSource::LightSource() {
    m_modelMatrix = QMatrix4x4();
    m_modelMatrix.setToIdentity();
    m_modelMatrix.scale(0.2f);
    m_modelMatrix.translate(QVector3D(0.0f, 0.0f, 0.0f));

	setBrightness(1.0f);
    setVisibility(true);
}

void LightSource::setBrightness(float brightness) {
    m_brightness = brightness;
}

void LightSource::setVisibility(bool isVisible) {
    m_visible = isVisible;
}

QMatrix4x4 LightSource::getModelMatrix() const {
    return m_modelMatrix;
}

float LightSource::getBrightness() const {
    return m_brightness;
}

bool LightSource::getVisibility() const {
    return m_visible;
}



} // namespace VDS