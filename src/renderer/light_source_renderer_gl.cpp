#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include "light_source_renderer_gl.h"

#include <QDebug>
#include <QMetaEnum>

#include <algorithm>
#include <string>

namespace VDS {

LightSourceRenderer::LightSourceRenderer(const QMatrix4x4* const projectionMatrix,
                                         const QMatrix4x4* const viewMatrix)
    : m_projectionMatrix(projectionMatrix), m_viewMatrix(viewMatrix) {}

LightSourceRenderer::~LightSourceRenderer() {}
void LightSourceRenderer::render() {
    glUseProgram(m_shaderProgram);

    // Bind vertex data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
    glBindVertexArray(m_vao_cube_vertices);

	// render all light sources without rebinding the shader programm each time
    for (const LightSource& light : m_lightSources) {
        const GLuint modelMatrixLocation = glGetUniformLocation(m_shaderProgram, "modelMatrix");
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, light.getModelMatrix().data());

        const GLuint brightnessLocation = glGetUniformLocation(m_shaderProgram, "brightness");
        glUniform1f(brightnessLocation, light.getBrightness());

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // Unbind vertex data
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // unbind shader programm
    glUseProgram(0);
}

void LightSourceRenderer::setup() {
    initializeOpenGLFunctions();

    setupBuffers();
    setupVertexArray();
    setupVertexShader();
    setupFragmentShader();
    setupShaderProgram();

    resetModelMatrix();
}

void LightSourceRenderer::resetModelMatrix() {
    m_rotationMatrix.setToIdentity();
    m_translationMatrix.setToIdentity();
    m_scaleMatrix.setToIdentity();
    applyMatrices();
}
void LightSourceRenderer::addLightSource(const LightSource& lightSource) {
    m_lightSources.push_back(lightSource);
}
void LightSourceRenderer::deleteLightSource(const LightSource& lightSource) {
    m_lightSources.erase(std::remove(m_lightSources.begin(), m_lightSources.end(), lightSource),
                         m_lightSources.end());
}
void LightSourceRenderer::applyMatrices() {
    glUseProgram(m_shaderProgram);
    {
        const GLuint projectionViewMatrixLocation =
            glGetUniformLocation(m_shaderProgram, "projectionViewMatrix");
        glUniformMatrix4fv(projectionViewMatrixLocation, 1, GL_FALSE,
                           (*m_projectionMatrix * *m_viewMatrix).data());

        const GLuint parentModelMatrixLocation =
            glGetUniformLocation(m_shaderProgram, "parentModelMatrix");
        glUniformMatrix4fv(parentModelMatrixLocation, 1, GL_FALSE,
                           (m_rotationMatrix * m_translationMatrix * m_scaleMatrix).data());
    }
    glUseProgram(0);
}
void LightSourceRenderer::rotate(float angle, float x, float y, float z) {
    m_rotationMatrix.rotate(angle, x, y, z);
    applyMatrices();
}
void LightSourceRenderer::translate(float x, float y, float z) {
    m_translationMatrix.translate(x, y, z);
    applyMatrices();
}
void LightSourceRenderer::scale(float factor) {
    m_scaleMatrix.scale(factor);
    applyMatrices();
}
const QMatrix4x4 LightSourceRenderer::getModelMatrix() const {
    return m_rotationMatrix * m_translationMatrix * m_scaleMatrix;
}
void LightSourceRenderer::setupBuffers() {
    GLfloat vertices[] = {
        // front
        -1.0f, -1.0f, 1.0f, // 0
        1.0f, -1.0f, 1.0f,  // 1
        1.0f, 1.0f, 1.0f,   // 2
        -1.0f, 1.0f, 1.0f,  // 3
        // back
        -1.0f, -1.0f, -1.0f, // 4
        1.0f, -1.0f, -1.0f,  // 5
        1.0f, 1.0f, -1.0f,   // 6
        -1.0f, 1.0f, -1.0    // 7
    };
    GLuint indices_cube[] = {// front
                             0, 1, 2, 2, 3, 0,
                             // right
                             1, 5, 6, 6, 2, 1,
                             // back
                             7, 6, 5, 5, 4, 7,
                             // left
                             4, 0, 3, 3, 7, 4,
                             // bottom
                             4, 5, 1, 1, 0, 4,
                             // top
                             3, 2, 6, 6, 7, 3};

    glGenBuffers(1, &m_vbo_cube_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_cube_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_ibo_cube_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_cube_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_cube), indices_cube, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void LightSourceRenderer::setupVertexArray() {
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
void LightSourceRenderer::setupVertexShader() {
    const GLchar* const vertexShaderSource =
        "#version 430 core \n"

        "in vec3 inPos; \n"
        "uniform mat4 projectionViewMatrix; \n"
        "uniform mat4 modelMatrix; \n"
        "uniform mat4 parentModelMatrix; \n"

        "void main() \n"
        "{ \n"
        "	gl_Position = projectionViewMatrix * parentModelMatrix * modelMatrix * vec4(inPos.x, "
        "inPos.y, inPos.z, 1.0f); \n"
        "} \n";

    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(m_vertexShader);
}

void LightSourceRenderer::setupFragmentShader() {
    const GLchar* const fragmentShaderSource = "#version 430 core \n"

                                               "uniform float brightness; \n"
                                               "out vec4 FragColor; \n"

                                               "void main() \n"
                                               "{ \n"
                                               "	FragColor = vec4(vec3(brightness), 1.0f); \n"
                                               "} \n";

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(m_fragmentShader);
}

void LightSourceRenderer::setupShaderProgram() {
    m_shaderProgram = glCreateProgram();

    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);
}

} // namespace VDS
