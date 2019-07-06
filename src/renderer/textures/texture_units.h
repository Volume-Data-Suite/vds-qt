#pragma once

#include <QOpenGLFunctions_4_3_Core>

namespace VDS
{
	enum class TextureUnits
	{
		VolumeData = GL_TEXTURE0,
		JitterNoise = GL_TEXTURE1,
		NormalData = GL_TEXTURE2,
	};
}