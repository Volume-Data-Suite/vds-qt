#pragma once

#include <array>
#include <vector>
#include <QOpenGLFunctions_4_3_Core>
#include <stdint.h>

namespace VDS {
class NoiseTexture2D : protected QOpenGLFunctions_4_3_Core {
public:
    NoiseTexture2D(uint8_t pow2);
    ~NoiseTexture2D();

    void setup();
    void updateNoise();

    uint32_t getSizeX() const;
    uint32_t getSizeY() const;

    GLuint getTextureHandle() const;

private:
    uint8_t m_pow2;
    GLuint m_texture;
};
} // namespace VDS