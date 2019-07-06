#pragma once

#include <QOpenGLFunctions_4_3_Core>
#include <array>
#include <vector>
#include <stdint.h>

namespace VDS
{
	class VolumeData3DTexture : protected QOpenGLFunctions_4_3_Core
	{
	public:
		VolumeData3DTexture();
		~VolumeData3DTexture();

		void setup();
		// need to call setup at least once before the first call of updateVolumeData
		void update(const std::array<uint32_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData);

		uint32_t getSizeX() const;
		uint32_t getSizeY() const;
		uint32_t getSizeZ() const;

		float getSpacingX() const;
		float getSpacingY() const;
		float getSpacingZ() const;

		GLuint getTextureHandle() const;

	private:

		std::array<uint32_t, 3> m_size;
		std::array<float, 3> m_spacing;
		GLuint m_texture;
	};
}