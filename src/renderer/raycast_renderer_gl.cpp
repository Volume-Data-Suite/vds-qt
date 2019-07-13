#include "raycast_renderer_gl.h"
#include "textures/texture_units.h"
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
		: m_projectionMatrix(projectionMatrix), m_viewMatrix(viewMatrix), m_noiseTexture(9)
	{
		m_sampleStepLength = 0.01f;
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

		const std::array<std::size_t, 3> volumeSize = { 1, 1, 1 };
		const std::array<float, 3> volumeSpacing = { 1.0f, 1.0f, 1.0f };
		m_texture.setup(volumeSize, volumeSpacing);
		m_noiseTexture.setup();

		// Resize volume to texture
		scaleVolumeAndNormalizeSize();

		updateSampleStepLength(m_sampleStepLength);

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
		// Bind noise texture
		glActiveTexture(GLenum(TextureUnits::JitterNoise));
		glBindTexture(GL_TEXTURE_2D, m_noiseTexture.getTextureHandle());

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		// Unbind noise texture
		glBindTexture(GL_TEXTURE_2D, 0);
		// Unbind volume data
		glBindTexture(GL_TEXTURE_3D, 0);

		//// reset active texture
		//glActiveTexture(GL_TEXTURE0);

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
		m_rotationMatrix.setToIdentity();
		m_translationMatrix.setToIdentity();
		m_scaleMatrix.setToIdentity();
		applyMatrices();
	}
	void RayCastRenderer::applyMatrices()
	{
		glUseProgram(m_shaderProgram);


		const QMatrix4x4 projectionViewModelMatrix = *m_projectionMatrix * *m_viewMatrix * (m_rotationMatrix * m_translationMatrix * m_scaleMatrix);
		const QMatrix4x4 viewModelMatrixWithoutModleScale = *m_viewMatrix * (m_rotationMatrix * m_translationMatrix);
			   
		const GLuint projectionViewModelMatrixID = glGetUniformLocation(m_shaderProgram, "projectionViewModelMatrix");
		const GLuint viewModelMatrixWithoutModleScaleID = glGetUniformLocation(m_shaderProgram, "viewModelMatrixWithoutModleScale");
		glUniformMatrix4fv(projectionViewModelMatrixID, 1, GL_FALSE, projectionViewModelMatrix.data());
		glUniformMatrix4fv(viewModelMatrixWithoutModleScaleID, 1, GL_FALSE, viewModelMatrixWithoutModleScale.data());


		const QVector3D rayOrigin = viewModelMatrixWithoutModleScale.inverted() * QVector3D({ 0.0, 0.0, 0.0 });

		const GLuint rayOriginID = glGetUniformLocation(m_shaderProgram, "rayOrigin");
		glUniform3f(rayOriginID, rayOrigin[0], rayOrigin[1], rayOrigin[2]);
	
		
		glUseProgram(0);

		updateFieldOfView();
	}
	void RayCastRenderer::rotate(float angle, float x, float y, float z)
	{
		m_rotationMatrix.rotate(angle, x, y, z);
		applyMatrices();
	}
	void RayCastRenderer::translate(float x, float y, float z)
	{
		m_translationMatrix.translate(x, y, z);
		applyMatrices();
	}
	void RayCastRenderer::scale(float factor)
	{
		m_scaleMatrix.scale(factor);
		applyMatrices();
	}
	void RayCastRenderer::updateVolumeData(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData)
	{
		m_texture.update(size, spacing, volumeData);

		// update texture for shader program
		glUseProgram(m_shaderProgram);
		// Bind volume data
		glBindTexture(GL_TEXTURE_3D, m_texture.getTextureHandle());
		glUniform1i(glGetUniformLocation(m_shaderProgram, "dataTex"), 0);
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

		glUseProgram(m_shaderProgram);

		const GLuint aspectRatioPosition = glGetUniformLocation(m_shaderProgram, "aspectRatio");
		glUniform1f(aspectRatioPosition, m_aspectRationOpenGLWindow);

		glUseProgram(0);
	}
	void RayCastRenderer::updateViewPortSize(int width, int heigth)
	{
		m_viewportSize[0] = static_cast<float>(width);
		m_viewportSize[1] = static_cast<float>(heigth);

		glUseProgram(m_shaderProgram);

		const GLuint viewPortSizePosition = glGetUniformLocation(m_shaderProgram, "viewportSize");
		glUniform2f(viewPortSizePosition, m_viewportSize[0], m_viewportSize[1]);

		glUseProgram(0);

		updateNoise();
	}
	void RayCastRenderer::updateSampleStepLength(float stepLength)
	{
		m_sampleStepLength = stepLength;

		glUseProgram(m_shaderProgram);

		const GLuint sampleStepLengthPosition = glGetUniformLocation(m_shaderProgram, "sampleStepLength");
		glUniform1f(sampleStepLengthPosition, m_sampleStepLength);

		glUseProgram(0);
	}
	const std::array<float, 3> RayCastRenderer::getPosition() const
	{
		const QMatrix4x4 modelMatrix = m_rotationMatrix * m_translationMatrix * m_scaleMatrix;

		// OpenGL is column major
		return std::array<float, 3>{
			modelMatrix.constData()[3 * 4 + 0],
			modelMatrix.constData()[3 * 4 + 1],
			modelMatrix.constData()[3 * 4 + 2]};
	}
	const QMatrix4x4 RayCastRenderer::getModelMatrix() const
	{
		return m_rotationMatrix * m_translationMatrix * m_scaleMatrix;
	}
	const float RayCastRenderer::getMinimalSampleStepLength() const
	{
		const std::size_t longestSide = std::max({m_texture.getSizeX(), m_texture.getSizeY(), m_texture.getSizeZ()});
		return 1.0f / static_cast<float>(longestSide);
	}
	void RayCastRenderer::updateFieldOfView()
	{
		const float projectionMatrixValue1x1 = m_projectionMatrix->constData()[1 * 4 + 1];
		const float fov = std::atan(1.0f / projectionMatrixValue1x1);
		const GLfloat focalLength = 1.0f / std::tan(fov);

		glUseProgram(m_shaderProgram);

		const GLuint focalPosition = glGetUniformLocation(m_shaderProgram, "focalLength");
		glUniform1f(focalPosition, focalLength);

		glUseProgram(0);
	}
	void RayCastRenderer::updateNoise()
	{
		m_noiseTexture.updateNoise();

		// update texture for shader program
		glUseProgram(m_shaderProgram);
		// Bind volume data
		glBindTexture(GL_TEXTURE_2D, m_noiseTexture.getTextureHandle());
		glUniform1i(glGetUniformLocation(m_shaderProgram, "noiseTex"), 1);
		// Unbind volume data
		glBindTexture(GL_TEXTURE_2D, 0);
		
		// unbind shader programm
		glUseProgram(0);
	}
	const QVector3D RayCastRenderer::getExtent() const
	{
		const float xInCentiMeter = m_texture.getSizeX() * m_texture.getSpacingX();
		const float yInCentiMeter = m_texture.getSizeY() * m_texture.getSpacingY();
		const float zInCentiMeter = m_texture.getSizeZ() * m_texture.getSpacingZ();

		const float longestSide = std::max({ xInCentiMeter, yInCentiMeter, zInCentiMeter });

		const float scaleX = xInCentiMeter / longestSide;
		const float scaleY = yInCentiMeter / longestSide;
		const float scaleZ = zInCentiMeter / longestSide;

		return QVector3D(scaleX, scaleY, scaleZ);
	}
	void RayCastRenderer::setupBuffers()
	{
		GLfloat vertices[] = {
			// front
			-1.0f,	-1.0f,	1.0f, // 0
			1.0f,	-1.0f,	1.0f, // 1
			1.0f,	1.0f,	1.0f, // 2
			-1.0f,	1.0f,	1.0f, // 3
			// back
			-1.0f,	-1.0f,	-1.0f, // 4
			1.0f,	-1.0f,	-1.0f, // 5
			1.0f,	1.0f,	-1.0f, // 6
			-1.0f,	1.0f,	-1.0  // 7
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
		
		bool compileStatus = checkShaderCompileStatus(m_vertexShader);

		if (!compileStatus)
		{
			qDebug() << vertexShaderSource.c_str();
		}

		return compileStatus;
	}

	bool RayCastRenderer::setupFragmentShader()
	{
		const std::string fragmentShaderSource = VDS::ShaderGenerator::getFragmentShaderCode(m_settings);
		const GLchar* const shaderGLSL = fragmentShaderSource.c_str();

		m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(m_fragmentShader, 1, &shaderGLSL, NULL);
		glCompileShader(m_fragmentShader);

		bool compileStatus = checkShaderCompileStatus(m_fragmentShader);

		if (!compileStatus)
		{
			qDebug() << fragmentShaderSource.c_str();
		}

		return compileStatus;
	}

	bool RayCastRenderer::setupShaderProgram()
	{
		m_shaderProgram = glCreateProgram();

		glAttachShader(m_shaderProgram, m_vertexShader);
		glAttachShader(m_shaderProgram, m_fragmentShader);
		glLinkProgram(m_shaderProgram);

		return checkShaderProgramLinkStatus(m_shaderProgram);
	}

	void RayCastRenderer::setAxisAlignedBoundingBox(const QVector3D& extent)
	{
		//float mini = std::min({ extent.x(), extent.y(), extent.z() });

		//QVector3D tmp = extent / mini;

		//QVector3D tmp(2, 2, 3);

		//const QVector3D top = tmp;
		//const QVector3D bottom = -tmp;

		const QVector3D top = extent;
		const QVector3D bottom = -extent;
		

		glUseProgram(m_shaderProgram);

		const GLuint topPosition = glGetUniformLocation(m_shaderProgram, "topAABB");
		glUniform3f(topPosition, top[0], top[1], top[2]);
		const GLuint bottomPosition = glGetUniformLocation(m_shaderProgram, "bottomAABB");
		glUniform3f(bottomPosition, bottom[0], bottom[1], bottom[2]);

		glUseProgram(0);
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
		const QVector3D extent = getExtent();

		m_scaleMatrix.scale(extent);
		applyMatrices();

		setAxisAlignedBoundingBox(extent);
	}
}
