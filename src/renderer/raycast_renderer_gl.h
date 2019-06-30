#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>


#include "volume_data_3D_texture.h"
#include <array>

namespace VDS {

	enum class RenderModes
	{
		Mesh,
		Borders,
	};

	enum class TextureUnits
	{
		VolumeData = GL_TEXTURE0,
		NormalData = GL_TEXTURE1,
	};

	class RayCastRenderer : public QObject, protected QOpenGLFunctions_4_3_Core
	{
		
	public:
		RayCastRenderer(const QMatrix4x4* const projectionMatrix, const QMatrix4x4* const viewMatrix);
		~RayCastRenderer();

		void render();

		bool setup();

		void applyMatrices();

		void rotate(float x, float y);
		void translate(float x, float y, float z);
		void resetModelMatrix();

		void updateVolumeData(const std::array<uint32_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData);

		// TODO: Dont need a function for that. get the data from projection matrix on projection matrix update
		void updateAspectRation(float ratio);

		void updateViewPortSize(int width, int heigth);


	private:
		void renderVolume();
		void renderMesh();
		void renderVolumeBorders();

		void setupBuffers();
		void setupVertexArray(RenderModes renderMode);
		bool setupVertexShader();
		bool setupFragmentShader();
		bool setupShaderProgram();

		bool checkShaderCompileStatus(GLuint shader);
		bool checkShaderProgramLinkStatus(GLuint shaderProgram);

		void scaleVolumeAndNormalizeSize();
		

		// global buffer handles
		GLuint m_vao_cube_vertices;
		GLuint m_vbo_cube_vertices;
		GLuint m_ibo_cube_elements;
		GLuint m_ibo_cube_lines_elements;
		// global shader hanldes
		GLuint m_vertexShader;
		GLuint m_fragmentShader;
		GLuint m_shaderProgram;

		// Matrices
		const QMatrix4x4* const m_projectionMatrix;
		const QMatrix4x4* const m_viewMatrix;
		QMatrix4x4 m_modelMatrix;

		// stores the volume data
		VolumeData3DTexture m_texture;

		float m_aspectRationOpenGLWindow;
		std::array<float, 2> m_viewportSize;
	};
}