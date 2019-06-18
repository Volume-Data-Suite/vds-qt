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
		m_position = QVector3D(0.0, -0.45, -0.0);
	}

	RayCastRenderer::~RayCastRenderer()
	{
	}
	void RayCastRenderer::render()
	{

		// set up vertex data (and buffer(s)) and configure vertex attributes
		// ------------------------------------------------------------------
		float vertices[] = {
			 1.0f,  1.0f, 0.0f,  // top right
			 1.0f, -1.0f, 0.0f,  // bottom right
			-1.0f, -1.0f, 0.0f,  // bottom left
			-1.0f,  1.0f, 0.0f   // top left 
		};
		unsigned int indices[] = {  // note that we start from 0!
			0, 1, 3,  // first Triangle
			1, 2, 3   // second Triangle
		};
		unsigned int VBO, VAO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(VAO);


		/*glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, m_cube_vertices.size(), m_cube_vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_cube_quad_indices.size(), m_cube_quad_indices.data(), GL_STATIC_DRAW);*/




		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);








		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		glBindVertexArray(0);


		// uncomment this call to draw in wireframe polygons.
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// render
			// ------

		// draw our first triangle
		glUseProgram(m_shaderProgram);
		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// glBindVertexArray(0); // no need to unbind it every time 




























		/*glUseProgram(m_shaderProgram);
		glBindVertexArray(m_vao_cube_vertices);
		glDrawArrays(GL_LINES, 0, 4);*/


		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
		//glDrawElements(GL_LINES, m_ibo_cube_elements, GL_UNSIGNED_SHORT, 0);


		// GL_QUADS
		/*glBegin(GL_POLYGON);
		glVertex3f(-0.5, -0.5, 0.5);
		glVertex3f(-0.5, 0.5, 0.5);
		glVertex3f(-0.5, 0.5, -0.5);
		glVertex3f(-0.5, -0.5, -0.5);
		glEnd();*/

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // this tells it to only render lines

		//glBegin(GL_LINES);
	}
	bool RayCastRenderer::setup()
	{
		initializeOpenGLFunctions();

		setupBuffers();

		if (!setupVertexShader() || !setupFragmentShader())
		{
			return false;
		}

		setupVertexArray();

		if (!setupShaderProgram())
		{
			return false;
		}

		setModelMatrix();
		applyMatrices();

		return true;
	}
	void RayCastRenderer::setModelMatrix()
	{
		m_modelMatrix.setToIdentity();
		m_modelMatrix.translate(m_position);
		//m_modelMatrix.rotate(m_rotation_angle, 0.0f, 0.0f, 0.0f);

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
	void RayCastRenderer::setupBuffers()
	{
		m_cube_vertices = {
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
		glGenBuffers(1, &m_vbo_cube_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_cube_vertices);
		glBufferData(GL_ARRAY_BUFFER, m_cube_vertices.size(), m_cube_vertices.data(), GL_STATIC_DRAW);



		// quads, counter-clockwise
		m_cube_quad_indices = {
			// front
			0, 1, 2, 3,
			// right
			1, 5, 6, 2,
			// back
			7, 6, 5, 4,
			// left
			4, 0, 3, 7,
			// bottom
			4, 5, 1, 0,
			// top
			3, 2, 6, 7
		};
		glGenBuffers(1, &m_ibo_cube_elements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_cube_quad_indices.size(), m_cube_quad_indices.data(), GL_STATIC_DRAW);



		// triangles
		//GLushort cube_elements[] = {
		//	// front
		//	0, 1, 2,
		//	2, 3, 0,
		//	// right
		//	1, 5, 6,
		//	6, 2, 1,
		//	// back
		//	7, 6, 5,
		//	5, 4, 7,
		//	// left
		//	4, 0, 3,
		//	3, 7, 4,
		//	// bottom
		//	4, 5, 1,
		//	1, 0, 4,
		//	// top
		//	3, 2, 6,
		//	6, 7, 3
		//};



		/*glGenBuffers(1, &m_ibo_cube_elements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
		const int cube_elements_size = sizeof(cube_elements) / sizeof(*cube_elements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_elements_size, cube_elements, GL_STATIC_DRAW);*/
	}

	void RayCastRenderer::setupVertexArray()
	{
		glGenVertexArrays(1, &m_vao_cube_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_cube_vertices);

		// set the vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);
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
