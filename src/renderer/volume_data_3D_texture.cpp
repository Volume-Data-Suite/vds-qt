#include "volume_data_3D_texture.h"


namespace VDS
{
	VolumeData3DTexture::VolumeData3DTexture(const VDTK::VolumeData & volumeData) 
		: m_texture { QOpenGLTexture::Target3D }, m_volumeData{volumeData}
	{
		updateTexture();
	}
	VolumeData3DTexture::~VolumeData3DTexture()
	{
	}
	void VolumeData3DTexture::updateVolumeData(const VDTK::VolumeData & volumeData)
	{
		m_volumeData = volumeData;
		updateTexture();
	}
	void VolumeData3DTexture::updateTexture()
	{
		m_texture.setMinificationFilter(QOpenGLTexture::Linear);
		m_texture.setMagnificationFilter(QOpenGLTexture::Linear);
		m_texture.setWrapMode(QOpenGLTexture::ClampToEdge);
		m_texture.setFormat(QOpenGLTexture::R16_UNorm);

		// TODO: figure out whats going on with swizzle
		const QOpenGLTexture::SwizzleValue red = QOpenGLTexture::RedValue;
		m_texture.setSwizzleMask(red, red, red, red);

		const int sizeX = static_cast<int>(m_volumeData.getSize().getX());
		const int sizeY = static_cast<int>(m_volumeData.getSize().getY());
		const int sizeZ = static_cast<int>(m_volumeData.getSize().getZ());
		m_texture.setSize(sizeX, sizeY, sizeZ);

		m_texture.allocateStorage();

		m_texture.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, m_volumeData.getRawVolumeData().data());

		m_texture.bind();
	}
}