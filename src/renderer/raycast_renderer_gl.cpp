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
		m_renderBoundingBox = false;
	}

	RayCastRenderer::~RayCastRenderer()
	{
	}
	void RayCastRenderer::render()
	{
		renderVolume();

		if (m_renderBoundingBox)
		{
			setupVertexArray(RenderModes::Borders);
			renderVolumeBorders();
			setupVertexArray(RenderModes::Mesh);
		}



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

		const std::array<std::size_t, 3> volumeSize = { 1, 1, 1 };
		const std::array<float, 3> volumeSpacing = { 1.0f, 1.0f, 1.0f };

		m_texture.setup(volumeSize, volumeSpacing);
		m_noiseTexture.setup();


		if (!generateRaycastShaderProgram())
		{
			return false;
		}
		
		if (!setupVertexShaderBoundingBox() || !setupFragmentShaderBoundingBox())
		{
			return false;
		}

		if (!setupShaderProgramBoundingBox())
		{
			return false;
		}

		resetModelMatrix();

		setBoundingBoxColor({ 1.0f, 1.0f, 1.0f, 1.0f });

		return true;
	}

	void RayCastRenderer::renderVolume()
	{
		glUseProgram(m_shaderProgramRayCasting);

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
		
		// Unbind vertex data
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// unbind shader programm
		glUseProgram(0);
	}
	void RayCastRenderer::renderMesh()
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(m_shaderProgramBoundingBox);

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
		glUseProgram(m_shaderProgramBoundingBox);

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
		const QMatrix4x4 projectionViewModelMatrix = *m_projectionMatrix * *m_viewMatrix * (m_rotationMatrix * m_translationMatrix * m_scaleMatrix);
		const QMatrix4x4 viewModelMatrixWithoutModleScale = *m_viewMatrix * (m_rotationMatrix * m_translationMatrix);
		
		glUseProgram(m_shaderProgramBoundingBox);
		{
			const GLuint projectionViewModelMatrixID = glGetUniformLocation(m_shaderProgramRayCasting, "projectionViewModelMatrix");
			const GLuint viewModelMatrixWithoutModleScaleID = glGetUniformLocation(m_shaderProgramRayCasting, "viewModelMatrixWithoutModleScale");
			glUniformMatrix4fv(projectionViewModelMatrixID, 1, GL_FALSE, projectionViewModelMatrix.data());
		}
		glUseProgram(0);


		glUseProgram(m_shaderProgramRayCasting);
		{
			const GLuint projectionViewModelMatrixID = glGetUniformLocation(m_shaderProgramRayCasting, "projectionViewModelMatrix");
			const GLuint viewModelMatrixWithoutModleScaleID = glGetUniformLocation(m_shaderProgramRayCasting, "viewModelMatrixWithoutModleScale");
			glUniformMatrix4fv(projectionViewModelMatrixID, 1, GL_FALSE, projectionViewModelMatrix.data());
			glUniformMatrix4fv(viewModelMatrixWithoutModleScaleID, 1, GL_FALSE, viewModelMatrixWithoutModleScale.data());

			const QVector3D rayOrigin = viewModelMatrixWithoutModleScale.inverted() * QVector3D({ 0.0, 0.0, 0.0 });

			const GLuint rayOriginID = glGetUniformLocation(m_shaderProgramRayCasting, "rayOrigin");
			glUniform3f(rayOriginID, rayOrigin[0], rayOrigin[1], rayOrigin[2]);
		}
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
		glUseProgram(m_shaderProgramRayCasting);
		// Bind volume data
		glBindTexture(GL_TEXTURE_3D, m_texture.getTextureHandle());
		glUniform1i(glGetUniformLocation(m_shaderProgramRayCasting, "dataTex"), 0);
		// Unbind volume data
		glBindTexture(GL_TEXTURE_3D, 0);
		
		// unbind shader programm
		glUseProgram(0);

		resetModelMatrix();

		// Resize volume box
		scaleVolumeAndNormalizeSize();
	}
	void RayCastRenderer::updateAspectRation(float ratio)
	{
		m_settings.aspectRationOpenGLWindow = ratio;

		glUseProgram(m_shaderProgramRayCasting);

		const GLuint aspectRatioPosition = glGetUniformLocation(m_shaderProgramRayCasting, "aspectRatio");
		glUniform1f(aspectRatioPosition, m_settings.aspectRationOpenGLWindow);

		glUseProgram(0);
	}
	void RayCastRenderer::updateViewPortSize(float width, float heigth)
	{
		m_settings.viewportSize[0] = width;
		m_settings.viewportSize[1] = heigth;

		glUseProgram(m_shaderProgramRayCasting);

		const GLuint viewPortSizePosition = glGetUniformLocation(m_shaderProgramRayCasting, "viewportSize");
		glUniform2f(viewPortSizePosition, m_settings.viewportSize[0], m_settings.viewportSize[1]);

		glUseProgram(0);

		updateNoise();
	}
	void RayCastRenderer::updateSampleStepLength(float stepLength)
	{
		m_settings.sampleStepLength = stepLength;

		glUseProgram(m_shaderProgramRayCasting);

		const GLuint sampleStepLengthPosition = glGetUniformLocation(m_shaderProgramRayCasting, "sampleStepLength");
		glUniform1f(sampleStepLengthPosition, m_settings.sampleStepLength);

		glUseProgram(0);
	}
	void RayCastRenderer::updateThreshold(float threshold)
	{
		m_settings.threshold = threshold;

		glUseProgram(m_shaderProgramRayCasting);

		const GLuint thresholdPosition = glGetUniformLocation(m_shaderProgramRayCasting, "threshold");
		glUniform1f(thresholdPosition, m_settings.threshold);

		glUseProgram(0);
	}
	void RayCastRenderer::applyValueWindow(bool active)
	{
		m_settings.windowSettings.enabled = active;

		generateRaycastShaderProgram();
	}
	void RayCastRenderer::updateValueWindowWidth(float windowWidth)
	{
		m_settings.windowSettings.valueWindowWidth = windowWidth;

		glUseProgram(m_shaderProgramRayCasting);

		const GLuint windowWidthPosition = glGetUniformLocation(m_shaderProgramRayCasting, "valueWindowWidth");
		glUniform1f(windowWidthPosition, m_settings.windowSettings.valueWindowWidth);

		glUseProgram(0);
	}
	void RayCastRenderer::updateValueWindowCenter(float windowCenter)
	{
		m_settings.windowSettings.valueWindowCenter = windowCenter;

		glUseProgram(m_shaderProgramRayCasting);

		const GLuint windowCenterPosition = glGetUniformLocation(m_shaderProgramRayCasting, "valueWindowCenter");
		glUniform1f(windowCenterPosition, m_settings.windowSettings.valueWindowCenter);

		glUseProgram(0);
	}
	void RayCastRenderer::updateValueWindowOffset(float windowOffset)
	{
		m_settings.windowSettings.valueWindowOffset = windowOffset;

		glUseProgram(m_shaderProgramRayCasting);

		const GLuint windowOffsetPosition = glGetUniformLocation(m_shaderProgramRayCasting, "valueWindowOffset");
		glUniform1f(windowOffsetPosition, m_settings.windowSettings.valueWindowOffset);

		glUseProgram(0);
	}
	void RayCastRenderer::setRayCastMethod(int method)
	{
		m_settings.method = static_cast<RayCastMethods>(method);
		generateRaycastShaderProgram();
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
	void RayCastRenderer::setBoundingBoxColor(const std::array<float, 4>& color)
	{
		glUseProgram(m_shaderProgramBoundingBox);

		const GLuint boundingBoxColorPosition = glGetUniformLocation(m_shaderProgramBoundingBox, "boundingBoxColor");
		glUniform4f(boundingBoxColorPosition, color[0], color[1], color[2], color[3]);

		glUseProgram(0);
	}
	void RayCastRenderer::setBoundingBoxRenderStatus(bool active)
	{
		m_renderBoundingBox = active;
	}
	void RayCastRenderer::updateFieldOfView()
	{
		const float projectionMatrixValue1x1 = m_projectionMatrix->constData()[1 * 4 + 1];
		const float fov = std::atan(1.0f / projectionMatrixValue1x1);
		const GLfloat focalLength = 1.0f / std::tan(fov);

		glUseProgram(m_shaderProgramRayCasting);

		const GLuint focalPosition = glGetUniformLocation(m_shaderProgramRayCasting, "focalLength");
		glUniform1f(focalPosition, focalLength);

		glUseProgram(0);
	}
	void RayCastRenderer::updateNoise()
	{
		m_noiseTexture.updateNoise();

		// update texture for shader program
		glUseProgram(m_shaderProgramRayCasting);
		// Bind volume data
		glBindTexture(GL_TEXTURE_2D, m_noiseTexture.getTextureHandle());
		glUniform1i(glGetUniformLocation(m_shaderProgramRayCasting, "noiseTex"), 1);
		// Unbind volume data
		glBindTexture(GL_TEXTURE_2D, 0);
		
		// unbind shader programm
		glUseProgram(0);
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


	bool RayCastRenderer::setupVertexShaderRayCasting()
	{
		const std::string vertexShaderSource = VDS::ShaderGenerator::getVertexShaderCode();
		const GLchar* const shaderGLSL = 	vertexShaderSource.c_str();

		m_vertexShaderRayCasting = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(m_vertexShaderRayCasting, 1, &shaderGLSL, NULL);
		glCompileShader(m_vertexShaderRayCasting);
		
		bool compileStatus = checkShaderCompileStatus(m_vertexShaderRayCasting);

		if (!compileStatus)
		{
			qDebug() << vertexShaderSource.c_str();
		}

		return compileStatus;
	}

	bool RayCastRenderer::setupFragmentShaderRayCasting()
	{
		const std::string fragmentShaderSource = VDS::ShaderGenerator::getFragmentShaderCode(m_settings);
		const GLchar* const shaderGLSL = fragmentShaderSource.c_str();

		m_fragmentShaderRayCasting = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(m_fragmentShaderRayCasting, 1, &shaderGLSL, NULL);
		glCompileShader(m_fragmentShaderRayCasting);

		bool compileStatus = checkShaderCompileStatus(m_fragmentShaderRayCasting);

		if (!compileStatus)
		{
			qDebug() << fragmentShaderSource.c_str();
		}

		return compileStatus;
	}

	bool RayCastRenderer::setupShaderProgramRayCasting()
	{
		m_shaderProgramRayCasting = glCreateProgram();

		glAttachShader(m_shaderProgramRayCasting, m_vertexShaderRayCasting);
		glAttachShader(m_shaderProgramRayCasting, m_fragmentShaderRayCasting);
		glLinkProgram(m_shaderProgramRayCasting);

		return checkShaderProgramLinkStatus(m_shaderProgramRayCasting);
	}

	bool RayCastRenderer::generateRaycastShaderProgram()
	{
		if (!setupVertexShaderRayCasting() || !setupFragmentShaderRayCasting())
		{
			return false;
		}

		if (!setupShaderProgramRayCasting())
		{
			return false;
		}

		applyMatrices();
		
		// Resize volume to texture
		scaleVolumeAndNormalizeSize();

		updateAspectRation(m_settings.aspectRationOpenGLWindow);
		updateViewPortSize(m_settings.viewportSize[0], m_settings.viewportSize[1]);
		updateFieldOfView();
		updateSampleStepLength(m_settings.sampleStepLength);
		updateThreshold(m_settings.threshold);
		updateValueWindowWidth(m_settings.windowSettings.valueWindowWidth);
		updateValueWindowCenter(m_settings.windowSettings.valueWindowCenter);
		updateValueWindowOffset(m_settings.windowSettings.valueWindowOffset);

		updateSampleStepLength(m_settings.sampleStepLength);

		return true;
	}

	bool RayCastRenderer::setupVertexShaderBoundingBox()
	{
		const GLchar* const vertexShaderSource =
			"#version 430 core \n"

			"in vec3 inPos; \n"
			"uniform mat4 projectionViewModelMatrix; \n"

			"void main() \n"
			"{ \n"
			"	gl_Position = projectionViewModelMatrix * vec4(inPos.x, inPos.y, inPos.z, 1.0f); \n"
			"} \n";

		m_vertexShaderBoundingBox = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(m_vertexShaderBoundingBox, 1, &vertexShaderSource, NULL);
		glCompileShader(m_vertexShaderBoundingBox);

		bool compileStatus = checkShaderCompileStatus(m_vertexShaderBoundingBox);

		if (!compileStatus)
		{
			qDebug() << vertexShaderSource;
		}

		return compileStatus;
	}

	bool RayCastRenderer::setupFragmentShaderBoundingBox()
	{
		const GLchar* const fragmentShaderSource =
			"#version 430 core \n"

			"uniform vec4 boundingBoxColor; \n"
			"out vec4 FragColor; \n"

			"void main() \n"
			"{ \n"
			"	FragColor = boundingBoxColor; \n"
			"} \n";

		m_fragmentShaderBoundingBox = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(m_fragmentShaderBoundingBox, 1, &fragmentShaderSource, NULL);
		glCompileShader(m_fragmentShaderBoundingBox);

		bool compileStatus = checkShaderCompileStatus(m_fragmentShaderBoundingBox);

		if (!compileStatus)
		{
			qDebug() << fragmentShaderSource;
		}

		return compileStatus;
	}

	bool RayCastRenderer::setupShaderProgramBoundingBox()
	{
		m_shaderProgramBoundingBox = glCreateProgram();

		glAttachShader(m_shaderProgramBoundingBox, m_vertexShaderBoundingBox);
		glAttachShader(m_shaderProgramBoundingBox, m_fragmentShaderBoundingBox);
		glLinkProgram(m_shaderProgramBoundingBox);
		
		return checkShaderProgramLinkStatus(m_shaderProgramBoundingBox);
	}

	void RayCastRenderer::setAxisAlignedBoundingBox(const std::array<float, 3>& extent)
	{
		glUseProgram(m_shaderProgramRayCasting);

		const GLuint topPosition = glGetUniformLocation(m_shaderProgramRayCasting, "topAABB");
		glUniform3f(topPosition, extent[0], extent[1], extent[2]);
		const GLuint bottomPosition = glGetUniformLocation(m_shaderProgramRayCasting, "bottomAABB");
		glUniform3f(bottomPosition, -extent[0], -extent[1], -extent[2]);

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
		const std::array<float, 3> extent = m_texture.getExtent();

		m_scaleMatrix.setToIdentity();
		m_scaleMatrix.scale(extent[0], extent[1], extent[2]);
		applyMatrices();

		setAxisAlignedBoundingBox(extent);
	}
}
