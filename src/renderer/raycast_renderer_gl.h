#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>


#include "textures/volume_data_3D_texture.h"
#include "textures/noise_texture_2D.h"
#include "shader/shader_generator.h"
#include <array>

namespace VDS {

	enum class RenderModes
	{
		Mesh,
		Borders,
	};


	class RayCastRenderer : public QObject, protected QOpenGLFunctions_4_3_Core
	{
		
	public:
		RayCastRenderer(const QMatrix4x4* const projectionMatrix, const QMatrix4x4* const viewMatrix);
		~RayCastRenderer();

		void render();

		bool setup();

		void applyMatrices();

		void rotate(float angle, float x, float y, float z);
		void translate(float x, float y, float z);
		void scale(float factor);
		void resetModelMatrix();

		void updateVolumeData(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData);

		// TODO: Dont need a function for that. get the data from projection matrix on projection matrix update
		void updateAspectRation(float ratio);

		void updateViewPortSize(int width, int heigth);

		void updateSampleStepLength(float stepLength);

		const std::array<float, 3> getPosition() const;
		const QMatrix4x4 getModelMatrix() const;
		// returns the sample step length which is required, if we do not want to skip any voxels
		const float getMinimalSampleStepLength() const;

	private:
		void renderVolume();
		void renderMesh();
		void renderVolumeBorders();

		void setupBuffers();
		void setupVertexArray(RenderModes renderMode);
		bool setupVertexShader();
		bool setupFragmentShader();
		bool setupShaderProgram();
		void setAxisAlignedBoundingBox(const QVector3D& extent);

		bool checkShaderCompileStatus(GLuint shader);
		bool checkShaderProgramLinkStatus(GLuint shaderProgram);

		void scaleVolumeAndNormalizeSize();

		void updateFieldOfView();
		void updateNoise();
		
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

		// stores random jitter noise
		NoiseTexture2D m_noiseTexture;

		float m_aspectRationOpenGLWindow;
		std::array<float, 2> m_viewportSize;

		float m_sampleStepLength;

		RaycastShaderSettings m_settings;
	};
}