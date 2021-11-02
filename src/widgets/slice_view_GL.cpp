#pragma once
#include "slice_view_GL.h"
#include "renderer/shader/shader_generator.h"

#include <algorithm>
#include <QtConcurrent>
#include <QFuture>


SliceViewGL::SliceViewGL(QWidget* parent)
    : QOpenGLWidget(parent) {
    is_opengl_initialized = false;
}

void SliceViewGL::setAxis(VDTK::VolumeAxis axis) {
    m_settings.axis = axis;
}

void SliceViewGL::setPosition(int position) {
    // sliders start with index 1 but texture position index starts with 0
    m_settings.position = position - 1;

    float texturePosition = 0.0;

    switch (m_settings.axis) {
    case VDTK::VolumeAxis::XYAxis:
        texturePosition = static_cast<float>(position) / static_cast<float>(m_settings.size.getZ());
        break;
    case VDTK::VolumeAxis::XZAxis:
        texturePosition = static_cast<float>(position) / static_cast<float>(m_settings.size.getY());
        break;
    case VDTK::VolumeAxis::YZAxis:
        texturePosition = static_cast<float>(position) / static_cast<float>(m_settings.size.getX());
        break;
    default:
        break;
    }

    glUseProgram(m_shaderProgram);

    const GLuint shaderPosition = glGetUniformLocation(m_shaderProgram, "position");
    glUniform1f(shaderPosition, texturePosition);

    glUseProgram(0);

    update();
}

void SliceViewGL::setSize(VDTK::VolumeSize size) {
    m_settings.size = size;
}

void SliceViewGL::applyValueWindow(bool active) {
    m_settings.windowSettings.enabled = active;

    generateShaderProgram();
}
void SliceViewGL::setValueWindowMethod(int method) {
    m_settings.windowSettings.method = VDS::WindowingMethod(method);

    generateShaderProgram();
}
void SliceViewGL::updateValueWindowWidth(float windowWidth) {
    m_settings.windowSettings.valueWindowWidth = windowWidth;

    glUseProgram(m_shaderProgram);

    const GLuint windowWidthPosition = glGetUniformLocation(m_shaderProgram, "valueWindowWidth");
    glUniform1f(windowWidthPosition, m_settings.windowSettings.valueWindowWidth);

    glUseProgram(0);

    update();
}
void SliceViewGL::updateValueWindowCenter(float windowCenter) {
    m_settings.windowSettings.valueWindowCenter = windowCenter;

    glUseProgram(m_shaderProgram);

    const GLuint windowCenterPosition = glGetUniformLocation(m_shaderProgram, "valueWindowCenter");
    glUniform1f(windowCenterPosition, m_settings.windowSettings.valueWindowCenter);

    glUseProgram(0);

    update();
}
void SliceViewGL::updateValueWindowOffset(float windowOffset) {
    m_settings.windowSettings.valueWindowOffset = windowOffset;

    glUseProgram(m_shaderProgram);

    const GLuint windowOffsetPosition = glGetUniformLocation(m_shaderProgram, "valueWindowOffset");
    glUniform1f(windowOffsetPosition, m_settings.windowSettings.valueWindowOffset);

    glUseProgram(0);

    update();
}

void SliceViewGL::initializeGL() {
    initializeOpenGLFunctions();
    is_opengl_initialized = true;

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
    generateShaderProgram();
}

void SliceViewGL::resizeGL(int w, int h) {
    updateViewPortSize(w, h);
}

void SliceViewGL::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shaderProgram);

    // Bind vertex data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBindVertexArray(m_vao);

    glBindTexture(GL_TEXTURE_3D, m_texture);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_3D, 0);

    // Unbind vertex data
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // unbind shader programm
    glUseProgram(0);
}

void SliceViewGL::enterEvent(QEvent* ev) {
    emit(enterEventSignaled());
}

void SliceViewGL::leaveEvent(QEvent* ev) {
    emit(leaveEventSignaled());
}

void SliceViewGL::updateTexture(GLuint texture) {
    if (!is_opengl_initialized) {
        return;
    }

    m_texture = texture;

    update();
}

void SliceViewGL::setupBuffers() {
    GLfloat vertices[] = {
        -1.0f, -1.0f, 0.0f, // 0
        3.0f,  -1.0f, 0.0f, // 1
        -1.0f, 3.0f,  0.0f, // 2
    };
    GLuint indices_cube[] = {// front
                             0, 1, 2};

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_cube), indices_cube, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SliceViewGL::setupVertexArray() {
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

bool SliceViewGL::setupVertexShader() {
    const std::string vertexShaderSource = VDS::ShaderGenerator::getVertexShaderCodeSlice2D();
    const GLchar* const shaderGLSL = vertexShaderSource.c_str();

    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &shaderGLSL, NULL);
    glCompileShader(m_vertexShader);

    bool compileStatus = checkShaderCompileStatus(m_vertexShader);

    if (!compileStatus) {
        qDebug() << vertexShaderSource.c_str();
    }

    return compileStatus;
}

bool SliceViewGL::setupFragmentShader() {
    const std::string fragmenntShaderSource = VDS::ShaderGenerator::getFragmentShaderCodeSlice2D(m_settings);
    const GLchar* const shaderGLSL = fragmenntShaderSource.c_str();

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &shaderGLSL, NULL);
    glCompileShader(m_fragmentShader);

    bool compileStatus = checkShaderCompileStatus(m_fragmentShader);

    if (!false) {
        qDebug() << fragmenntShaderSource.c_str();
    }

    return compileStatus;
}

bool SliceViewGL::setupShaderProgram() {
    m_shaderProgram = glCreateProgram();

    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);

    return checkShaderProgramLinkStatus(m_shaderProgram);
}

void SliceViewGL::updateViewPortSize(float width, float heigth) {
    m_settings.viewportSize[0] = width;
    m_settings.viewportSize[1] = heigth;

    glViewport(0, 0, m_settings.viewportSize[0], m_settings.viewportSize[1]);

    glUseProgram(m_shaderProgram);
    const GLuint viewport = glGetUniformLocation(m_shaderProgram, "viewport");
    glUniform2f(viewport, static_cast<float>(m_settings.viewportSize[0]),
                static_cast<float>(m_settings.viewportSize[1]));

    glUseProgram(0);
}

void SliceViewGL::updateThreshold(float threshold) {
    m_settings.threshold = threshold / 1000.0f;

    glUseProgram(m_shaderProgram);

    const GLuint thresholdPosition = glGetUniformLocation(m_shaderProgram, "threshold");
    glUniform1f(thresholdPosition, m_settings.threshold);

    glUseProgram(0);

    update();
}

bool SliceViewGL::generateShaderProgram() {
    if (!setupVertexShader() || !setupFragmentShader()) {
        return false;
    }

    if (!setupShaderProgram()) {
        return false;
    }

    updateShaderUniforms();

    return true;
}

void SliceViewGL::updateShaderUniforms() {
    updateViewPortSize(m_settings.viewportSize[0], m_settings.viewportSize[1]);
    updateThreshold(m_settings.threshold);
    setPosition(m_settings.position);
    updateValueWindowWidth(m_settings.windowSettings.valueWindowWidth);
    updateValueWindowCenter(m_settings.windowSettings.valueWindowCenter);
    updateValueWindowOffset(m_settings.windowSettings.valueWindowOffset);
    update();
}

bool SliceViewGL::checkShaderCompileStatus(GLuint shader) {
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(shader); // Don't leak the shader.

        // Log error
        qDebug() << errorLog.data();
    }

    return isCompiled;
}
bool SliceViewGL::checkShaderProgramLinkStatus(GLuint shaderProgram) {
    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &errorLog[0]);

        // The program is useless now. So delete it.
        glDeleteProgram(shaderProgram);

        // Log error
        qDebug() << errorLog.data();
    }

    return isLinked;
}
