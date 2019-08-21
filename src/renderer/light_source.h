#pragma once

#include <array>
#include <QMatrix4x4>

namespace VDS {

class LightSource {
public:
    LightSource();
    ~LightSource() = default;

    void setBrightness(float brightness);
    void setVisibility(bool isVisible);

    QMatrix4x4 getModelMatrix() const;
    float getBrightness() const;
    bool getVisibility() const;

    bool operator==(const LightSource& other) const {

        return m_modelMatrix == other.m_modelMatrix && this->m_brightness == other.m_brightness &&
               this->m_visible == other.m_visible;
    }

private:
    QMatrix4x4 m_modelMatrix;
    float m_brightness;
    bool m_visible;
};

} // namespace VDS