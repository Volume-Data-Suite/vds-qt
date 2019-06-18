#include "raycast_renderer_gl.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>


#include <QDebug>
#include <QMetaEnum>

namespace VDS {

	RayCastRenderer::RayCastRenderer(const QMatrix4x4* const projectionMatrix, const QMatrix4x4* const viewMatrix)
		: m_projectionMatrix(projectionMatrix), m_viewMatrix(viewMatrix)
	{
	}

	RayCastRenderer::~RayCastRenderer()
	{
	}
	void RayCastRenderer::render()
	{
		// uncomment this call to draw in wireframe polygons.
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		

		glUseProgram(m_shaderProgram);

		// Bind vertex data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
		glBindVertexArray(m_vao_cube_vertices);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Unbind vertex data
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	bool RayCastRenderer::setup()
	{
		initializeOpenGLFunctions();

		setupBuffers();
		setupVertexArray();

		if (!setupVertexShader() || !setupFragmentShader())
		{
			return false;
		}
		
		if (!setupShaderProgram())
		{
			return false;
		}

		resetModelMatrix();

		return true;
	}
	void RayCastRenderer::resetModelMatrix()
	{
		m_modelMatrix.setToIdentity();
		applyMatrices();
	}
	void RayCastRenderer::applyMatrices()
	{
		glUseProgram(m_shaderProgram);

		const GLuint modelMatrixID = glGetUniformLocation(m_shaderProgram, "mMatrix");
		const GLuint viewMatrixID = glGetUniformLocation(m_shaderProgram, "vMatrix");
		const GLuint projectionMatrixID = glGetUniformLocation(m_shaderProgram, "pMatrix");

		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, m_modelMatrix.data());
		glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, m_viewMatrix->data());
		glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, m_projectionMatrix->data());

		checkShaderProgramLinkStatus(m_shaderProgram);
	}
	void RayCastRenderer::rotate(float x, float y)
	{
		const QVector3D rotation = QVector3D(x, y, 0.0f);
		m_modelMatrix.rotate(rotation.length(), rotation);
		applyMatrices();
	}
	void RayCastRenderer::translate(float x, float y, float z)
	{
		m_modelMatrix.translate(x, y, z);
	}
	void RayCastRenderer::setupBuffers()
	{
		GLfloat vertices[] = {
			 0.5f,  0.5f, 0.0f,  // top right
			 0.5f, -0.5f, 0.0f,  // bottom right
			-0.5f, -0.5f, 0.0f,  // bottom left
			-0.5f,  0.5f, 0.0f   // top left 
		};
		GLuint indices[] = {
			0, 1, 3,  // first Triangle
			1, 2, 3   // second Triangle
		};


		glGenBuffers(1, &m_vbo_cube_vertices);		
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_cube_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &m_ibo_cube_elements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		
		// unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void RayCastRenderer::setupVertexArray()
	{
		glGenVertexArrays(1, &m_vao_cube_vertices);

		glBindVertexArray(m_vao_cube_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_cube_vertices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);

		// set the vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);

		// unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}


	bool RayCastRenderer::setupVertexShader()
	{
		constexpr char* vertexShaderSource =
			"#version 450 core \n"

			"layout(location = 0) in vec3 inPos; \n"
			"// Model Matrix \n"
			"layout(location = 1) uniform mat4 mMatrix; \n"
			"// View Matrix \n"
			"layout(location = 2) uniform mat4 vMatrix; \n"
			"// Projection Matrix \n"
			"layout(location = 3) uniform mat4 pMatrix; \n"

			"void main() \n"
			"{ \n"
			"	gl_Position = pMatrix * vMatrix * mMatrix * vec4(inPos.x, inPos.y, inPos.z, 1.0f); \n"
			"} \n";


		m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(m_vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(m_vertexShader);

		return checkShaderCompileStatus(m_vertexShader);
	}

	bool RayCastRenderer::setupFragmentShader()
	{
		constexpr char* fragmentShaderSource =
			"#version 450 core \n"

			"// uniform vec4 color; \n"
			"out vec4 FragColor; \n"

			"void main() \n"
			"{ \n"
			"	FragColor = vec4(1.0f); //color; \n"
			"} \n";


		m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(m_fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(m_fragmentShader);

		return checkShaderCompileStatus(m_fragmentShader);
	}

	bool RayCastRenderer::setupShaderProgram()
	{
		m_shaderProgram = glCreateProgram();

		glAttachShader(m_shaderProgram, m_vertexShader);
		glAttachShader(m_shaderProgram, m_fragmentShader);
		glLinkProgram(m_shaderProgram);

		return checkShaderProgramLinkStatus(m_shaderProgram);
	}

	bool RayCastRenderer::checkShaderCompileStatus(GLuint shader)
	{
		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

			// Provide the infolog in whatever manor you deem best.
			// Exit with failure.
			glDeleteShader(shader); // Don't leak the shader.

			// Log error
			qDebug(errorLog.data());
		}

		return isCompiled;
	}
	bool RayCastRenderer::checkShaderProgramLinkStatus(GLuint shaderProgram)
	{
		GLint isLinked = 0;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> errorLog(maxLength);
			glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &errorLog[0]);

			// The program is useless now. So delete it.
			glDeleteProgram(shaderProgram);

			// Log error
			qDebug(errorLog.data());
		}

		return isLinked;
	}
}
