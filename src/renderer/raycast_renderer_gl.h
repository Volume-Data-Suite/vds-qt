#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>

#include <QObject>

#include "volume_data_3D_texture.h"
#include <array>

namespace VDS {

	class RayCastRenderer : public QObject, protected QOpenGLFunctions_4_3_Core
	{

		Q_OBJECT

	public:
		RayCastRenderer(const QMatrix4x4* const projectionMatrix, const QMatrix4x4* const viewMatrix);
		~RayCastRenderer();

		void render();

		bool setup();

		void applyMatrices();

	private:
		void setModelMatrix();

		void setupBuffers();
		void setupVertexArray();
		bool setupVertexShader();
		bool setupFragmentShader();
		bool setupShaderProgram();

		bool checkShaderCompileStatus(GLuint shader);
		bool checkShaderProgramLinkStatus(GLuint shaderProgram);

		// buffers
		std::vector<GLfloat> m_cube_vertices;
		std::vector<GLushort> m_cube_quad_indices;

		// global buffer handles
		GLuint m_vao_cube_vertices;
		GLuint m_vbo_cube_vertices;
		GLuint m_ibo_cube_elements;
		// global shader hanldes
		GLuint m_vertexShader;
		GLuint m_fragmentShader;
		GLuint m_shaderProgram;

		// Matrices
		const QMatrix4x4* const m_projectionMatrix;
		const QMatrix4x4* const m_viewMatrix;
		QMatrix4x4 m_modelMatrix;

		// Volume properties
		QVector3D m_position;
		float m_rotation_angle;
	};
}