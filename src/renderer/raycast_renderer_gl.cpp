#include "raycast_renderer_gl.h"
#include "shader/shader_generator.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>


#include <QDebug>
#include <QMetaEnum>

#include <algorithm>
#include <string>

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
		//renderVolumeBorders();
		renderVolume();

#ifdef _DEBUG
		// check OpenGL error
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			qDebug() << "OpenGL error: " << err << endl;
		}
#endif // _DEBUG

	}

	bool RayCastRenderer::setup()
	{
		initializeOpenGLFunctions();

		setupBuffers();
		setupVertexArray(RenderModes::Mesh);

		if (!setupVertexShader() || !setupFragmentShader())
		{
			return false;
		}

		if (!setupShaderProgram())
		{
			return false;
		}

		resetModelMatrix();

		m_texture.setup();

		return true;
	}

	void RayCastRenderer::renderVolume()
	{
		glUseProgram(m_shaderProgram);

		// Bind vertex data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
		glBindVertexArray(m_vao_cube_vertices);

		// Bind volume data
		glActiveTexture(GLenum(TextureUnits::VolumeData));
		glBindTexture(GL_TEXTURE_3D, m_texture.getTextureHandle());

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// Unbind volume data
		glBindTexture(GL_TEXTURE_3D, 0);

		// Unbind vertex data
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// unbind shader programm
		glUseProgram(0);
	}
	void RayCastRenderer::renderMesh()
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(m_shaderProgram);

		// Bind vertex data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
		glBindVertexArray(m_vao_cube_vertices);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// Unbind vertex data
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// unbind shader programm
		glUseProgram(0);

		// reset ploygon mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	void RayCastRenderer::renderVolumeBorders()
	{
		glUseProgram(m_shaderProgram);

		// Bind vertex data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_lines_elements);
		glBindVertexArray(m_vao_cube_vertices);

		glDrawElements(GL_LINES, 36, GL_UNSIGNED_INT, 0);

		// Unbind vertex data
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// unbind shader programm
		glUseProgram(0);
	}
	void RayCastRenderer::resetModelMatrix()
	{
		m_modelMatrix.setToIdentity();
		applyMatrices();
	}
	void RayCastRenderer::applyMatrices()
	{
		glUseProgram(m_shaderProgram);

		const GLuint modelMatrixID = glGetUniformLocation(m_shaderProgram, "modelMatrix");
		const GLuint viewMatrixID = glGetUniformLocation(m_shaderProgram, "viewMatrix");
		const GLuint projectionMatrixID = glGetUniformLocation(m_shaderProgram, "projectionMatrix");

		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, m_modelMatrix.data());
		glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, m_viewMatrix->data());
		glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, m_projectionMatrix->data());






		// TODO use the same fov from the projection matirx
		// TODO better handle pi
		// TODO better handle focal length variable
		const GLfloat m_fov = 60.0f;
		constexpr double pi = 3.14159265358979323846;
		const GLfloat m_focalLength = 1.0 / std::tan(pi / 180.0 * m_fov / 2.0);
		const GLuint focalPosition = glGetUniformLocation(m_shaderProgram, "focal_length");
		glUniform1f(focalPosition, m_focalLength);


		const QMatrix4x4 test = *m_viewMatrix * m_modelMatrix;
		
		const QVector3D m_rayOrigin = test.inverted() * QVector3D({ 0.0, 0.0, 0.0 });
		const GLuint rayorigin = glGetUniformLocation(m_shaderProgram, "ray_origin");
		glUniform3f(rayorigin, m_rayOrigin[0], m_rayOrigin[1], m_rayOrigin[2]);


		

		QVector3D extent;
		extent.setX(m_texture.getSizeX() * m_texture.getSpacingX());
		extent.setY(m_texture.getSizeY() * m_texture.getSpacingY());
		extent.setZ(m_texture.getSizeZ() * m_texture.getSpacingZ());
		const float extentMax = std::max({extent.x(), extent.y(), extent.z()});
		extent.setX(extent.x() / extentMax);
		extent.setY(extent.y() / extentMax);
		extent.setZ(extent.z() / extentMax);

		QVector3D top = extent;
		QVector3D bottom = -extent;

		const GLuint topPosition = glGetUniformLocation(m_shaderProgram, "top");
		glUniform3f(topPosition, top[0], top[1], top[2]);
		const GLuint bottomPosition = glGetUniformLocation(m_shaderProgram, "bottom");
		glUniform3f(bottomPosition, bottom[0], bottom[1], bottom[2]);


		

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
		applyMatrices();
	}
	void RayCastRenderer::updateVolumeData(const std::array<uint32_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData)
	{
		m_texture.updateVolumeData(size, spacing, volumeData);

		// update texture for shader program
		glUseProgram(m_fragmentShader);
		// Bind volume data
		glBindTexture(GL_TEXTURE_3D, m_texture.getTextureHandle());
		glUniform1i(glGetUniformLocation(m_fragmentShader, "dataTex"), 0);
		// Unbind volume data
		glBindTexture(GL_TEXTURE_3D, 0);

		resetModelMatrix();


		// unbind shader programm
		glUseProgram(0);

		// Resize volume box
		scaleVolumeAndNormalizeSize();
	}
	void RayCastRenderer::updateAspectRation(float ratio)
	{
		m_aspectRationOpenGLWindow = ratio;

		const GLuint aspectRatioPosition = glGetUniformLocation(m_shaderProgram, "aspect_ratio");
		glUniform1f(aspectRatioPosition, m_aspectRationOpenGLWindow);
	}
	void RayCastRenderer::updateViewPortSize(int width, int heigth)
	{
		m_viewportSize[0] = static_cast<float>(width);
		m_viewportSize[1] = static_cast<float>(heigth);

		const GLuint viewPortSizePosition = glGetUniformLocation(m_shaderProgram, "viewport_size");
		glUniform2f(viewPortSizePosition, m_viewportSize[0], m_viewportSize[1]);
	}
	void RayCastRenderer::setupBuffers()
	{
		GLfloat vertices[] = {
			// front
			-1.0f, -1.0f,  1.0f, // 0
			1.0f, -1.0f,  1.0f, // 1
			1.0f,  1.0f,  1.0f, // 2
			-1.0f,  1.0f,  1.0f, // 3
			// back
			-1.0f, -1.0f, -1.0f, // 4
			1.0f, -1.0f, -1.0f, // 5
			1.0f,  1.0f, -1.0f, // 6
			-1.0f,  1.0f, -1.0  // 7
		};
		GLuint indices_cube[] = {
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// left
			4, 0, 3,
			3, 7, 4,
			// bottom
			4, 5, 1,
			1, 0, 4,
			// top
			3, 2, 6,
			6, 7, 3
		};
		GLuint indices_cube_lines[] = {
			// front
			0, 1,
			0, 3,
			2, 1,
			2, 3,

			// back
			4, 5,
			4, 7,
			6, 5,
			6, 7,

			// connect front an back
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		glGenBuffers(1, &m_vbo_cube_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_cube_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &m_ibo_cube_elements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_cube), indices_cube, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenBuffers(1, &m_ibo_cube_lines_elements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_lines_elements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_cube_lines), indices_cube_lines, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void RayCastRenderer::setupVertexArray(RenderModes renderMode)
	{
		GLuint ibo;

		switch (renderMode)
		{
		case VDS::RenderModes::Mesh:
			ibo = m_ibo_cube_elements;
			break;
		case VDS::RenderModes::Borders:
			ibo = m_ibo_cube_lines_elements;
			break;
		default:
			break;
		}

		glGenVertexArrays(1, &m_vao_cube_vertices);

		glBindVertexArray(m_vao_cube_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_cube_vertices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

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
		const std::string vertexShaderSource = VDS::ShaderGenerator::getVertexShaderCode();
		const GLchar* const shaderGLSL = 	vertexShaderSource.c_str();

		m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(m_vertexShader, 1, &shaderGLSL, NULL);
		glCompileShader(m_vertexShader);
		
		return checkShaderCompileStatus(m_vertexShader);
	}

	bool RayCastRenderer::setupFragmentShader()
	{
		const std::string fragmentShaderSource = VDS::ShaderGenerator::getFragmentShaderCode();
		const GLchar* const shaderGLSL = fragmentShaderSource.c_str();

		m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(m_fragmentShader, 1, &shaderGLSL, NULL);
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
	void RayCastRenderer::scaleVolumeAndNormalizeSize()
	{
		const float xInCentiMeter = m_texture.getSizeX() * m_texture.getSpacingX();
		const float yInCentiMeter = m_texture.getSizeY() * m_texture.getSpacingY();
		const float zInCentiMeter = m_texture.getSizeZ() * m_texture.getSpacingZ();

		const float longestSide = std::max({ xInCentiMeter, yInCentiMeter, zInCentiMeter });

		const float scaleX = xInCentiMeter / longestSide;
		const float scaleY = yInCentiMeter / longestSide;
		const float scaleZ = zInCentiMeter / longestSide;

		m_modelMatrix.scale(scaleX, scaleY, scaleZ);
		applyMatrices();
	}
}
