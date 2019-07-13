
#include "volume_data_3D_texture.h"



namespace VDS
{
	VolumeData3DTexture::VolumeData3DTexture() 
	{
		m_size = { 1, 1, 1};
		m_spacing = { 1.0f, 1.0f, 1.0f};
		m_texture = 0;
	}
	VolumeData3DTexture::~VolumeData3DTexture()
	{
		glDeleteTextures(1, &m_texture);
	}
	void VolumeData3DTexture::setup(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing)
	{
		m_size = size;
		m_spacing = spacing;

		// generate dummy data
		std::vector<uint16_t> dummyData = std::vector<uint16_t>(getSizeX() * getSizeY() * getSizeZ(), UINT16_MAX / 2);

		initializeOpenGLFunctions();

		glGenTextures(1, &m_texture);

		update(m_size, m_spacing, dummyData);
	}
	std::size_t VolumeData3DTexture::getSizeX() const
	{
		return m_size[0];
	}
	std::size_t VolumeData3DTexture::getSizeY() const
	{
		return m_size[1];
	}
	std::size_t VolumeData3DTexture::getSizeZ() const
	{
		return m_size[2];
	}
	float VolumeData3DTexture::getSpacingX() const
	{
		return m_spacing[0];
	}
	float VolumeData3DTexture::getSpacingY() const
	{
		return m_spacing[1];
	}
	float VolumeData3DTexture::getSpacingZ() const
	{
		return m_spacing[2];
	}
	GLuint VolumeData3DTexture::getTextureHandle() const
	{
		return m_texture;
	}
	void VolumeData3DTexture::update(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData)
	{
		m_size = size;
		m_spacing = spacing;

		glBindTexture(GL_TEXTURE_3D, m_texture);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, getSizeX(), getSizeY(), getSizeZ(), 0, GL_RED, GL_UNSIGNED_SHORT, volumeData.data());

		// unbind
		glBindTexture(GL_TEXTURE_3D, 0);
	}
}