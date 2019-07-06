
#include "noise_texture_2D.h"

#include <limits>
#include <random>

namespace VDS
{
	NoiseTexture2D::NoiseTexture2D(uint8_t pow2)
	{
		m_pow2 = pow2;
	}
	NoiseTexture2D::~NoiseTexture2D()
	{
		glDeleteTextures(1, &m_texture);
	}
	void NoiseTexture2D::setup()
	{
		initializeOpenGLFunctions();

		glDeleteTextures(1, &m_texture);
		glGenTextures(1, &m_texture);

		updateNoise();
	}
	uint32_t NoiseTexture2D::getSizeX() const
	{
		return std::pow(2, m_pow2) / 2;
	}
	uint32_t NoiseTexture2D::getSizeY() const
	{
		return std::pow(2, m_pow2) / 2;
	}
	GLuint NoiseTexture2D::getTextureHandle() const
	{
		return m_texture;
	}
	void NoiseTexture2D::updateNoise()
	{

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		const int width = viewport[2];
		const int height = viewport[3];


		const uint32_t size = width * height;

		static std::uniform_int_distribution<uint16_t> distribution(
			std::numeric_limits<uint16_t>::min(),
			std::numeric_limits<uint16_t>::max());
		static std::default_random_engine generator;

		std::vector<uint16_t> noise(size);
		

		std::generate(noise.begin(), noise.end(), []() { return distribution(generator); });



		glBindTexture(GL_TEXTURE_2D, m_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, noise.data());

		// unbind
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}