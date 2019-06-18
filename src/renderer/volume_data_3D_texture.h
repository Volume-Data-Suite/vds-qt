#pragma once

#include <QOpenGLTexture>

#include <VDTK/common/CommonDataTypes.h>

namespace VDS
{
	class VolumeData3DTexture
	{
	public:
		VolumeData3DTexture(const VDTK::VolumeData& volumeData);
		~VolumeData3DTexture();

		void updateVolumeData(const VDTK::VolumeData& volumeData);

	private:
		void updateTexture();

		QOpenGLTexture m_texture;
		VDTK::VolumeData m_volumeData;
	};
}