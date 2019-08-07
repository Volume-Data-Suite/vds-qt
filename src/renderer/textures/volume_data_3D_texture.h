#pragma once

#include <array>
#include <vector>
#include <QOpenGLFunctions_4_3_Core>
#include <stdint.h>

namespace VDS {
class VolumeData3DTexture : protected QOpenGLFunctions_4_3_Core {
public:
    VolumeData3DTexture();
    ~VolumeData3DTexture();

    void setup(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing);
    // need to call setup at least once before the first call of updateVolumeData
    void update(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing,
                const std::vector<uint16_t>& volumeData);

    std::size_t getSizeX() const;
    std::size_t getSizeY() const;
    std::size_t getSizeZ() const;

    float getSpacingX() const;
    float getSpacingY() const;
    float getSpacingZ() const;

    const std::array<float, 3> getExtent() const;

    GLuint getTextureHandle() const;

private:
    std::array<std::size_t, 3> m_size;
    std::array<float, 3> m_spacing;
    GLuint m_texture;
};
} // namespace VDS