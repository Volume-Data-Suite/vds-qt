#include "histogram_view_GL.h"


#include <algorithm>

HistogramViewGL::HistogramViewGL(QWidget *parent) : QOpenGLWidget(parent), m_histogramData(UINT16_MAX, 0)
{
}


void HistogramViewGL::updateHistogramData(const std::vector<uint16_t>& histo)
{
	m_histogramData = histo;
	m_max = *std::max_element(m_histogramData.begin(), m_histogramData.end());

	calculateScaledHistogram();
}

void HistogramViewGL::initializeGL()
{
	initializeOpenGLFunctions();
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); 
	glFrontFace(GL_CCW);

	glClearDepth(1.0f);
	// Change the reference of the GL_COLOR_BUFFER_BIT
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	setupBuffers();
	setupVertexArray();
	setupVertexShader();
	setupFragmentShader();
	setupShaderProgram();
}

void HistogramViewGL::resizeGL(int w, int h)
{
	m_width = w;
	m_height = h;
	glViewport(0, 0, m_width, m_height);
	calculateScaledHistogram();
}

void HistogramViewGL::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_shaderProgram);

	// Bind vertex data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBindVertexArray(m_vao);

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	
	// Unbind vertex data
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// unbind shader programm
	glUseProgram(0);
}

void HistogramViewGL::calculateScaledHistogram()
{
	m_histogramDataScaled.resize(m_width);

	const std::size_t size = m_histogramData.size();
	const std::size_t sectionSize = size / m_width;

	for (std::size_t index = 0; index < m_width; index ++)
	{
		m_histogramDataScaled[index] = *std::max_element(m_histogramData.begin() + index * sectionSize, m_histogramData.begin() + (index + 1) * sectionSize - 1);
	}
}

void HistogramViewGL::setupBuffers()
{
	GLfloat vertices[] = {
		-1.0f,	-1.0f,	-1.0f, // 0
		1.0f,	-1.0f,	-1.0f, // 1
		1.0f,	1.0f,	-1.0f, // 2
		-1.0f,	1.0f,	-1.0f // 3
	};
	GLuint indices_cube[] = {
		// front
		0, 1, 2,
		2, 3, 0
	};

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_cube), indices_cube, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void HistogramViewGL::setupVertexArray()
{
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	// set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void HistogramViewGL::setupVertexShader()
{
	const GLchar* const shaderGLSL = 
		"#version 430 core \n"

		"in vec3 inPos; \n"
		"uniform mat4 projectionViewModelMatrix; \n"

		"void main() \n"
		"{ \n"
		//"	gl_Position = projectionViewModelMatrix * vec4(inPos.x, inPos.y, inPos.z, 1.0f); \n"
		//"	gl_Position = vec4(inPos.x, inPos.y, inPos.z, 1.0f); \n"
		"	const float x = -1.0 + float((gl_VertexID & 1) << 2); \n"
		"	const float y = -1.0 + float((gl_VertexID & 2) << 1); \n"
		"	gl_Position =  vec4(x, y, 0.0f, 1.0f) \n"
		"} \n";

	m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_vertexShader, 1, &shaderGLSL, NULL);
	glCompileShader(m_vertexShader);
}

void HistogramViewGL::setupFragmentShader()
{
	const GLchar* const shaderGLSL = 
		"#version 430 core \n"

		"uniform vec2 viewPortSize; \n"
		"out vec4 FragColor; \n"

		"void main() \n"
		"{ \n"
		"	FragColor = vec4(1.0f); \n"
		"} \n";

	m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_fragmentShader, 1, &shaderGLSL, NULL);
	glCompileShader(m_fragmentShader);
}

void HistogramViewGL::setupShaderProgram()
{
	m_shaderProgram = glCreateProgram();

	glAttachShader(m_shaderProgram, m_vertexShader);
	glAttachShader(m_shaderProgram, m_fragmentShader);
	glLinkProgram(m_shaderProgram);
}
