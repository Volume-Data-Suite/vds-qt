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
    m_settings.position = position;

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
    setupVertexShader();
    setupFragmentShader();
    setupShaderProgram();
    setupTexture();
}

void SliceViewGL::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    glUseProgram(m_shaderProgram);
    const GLuint viewport = glGetUniformLocation(m_shaderProgram, "viewport");
    glUniform2f(viewport, static_cast<float>(w), static_cast<float>(h));
    glUseProgram(0);
}

void SliceViewGL::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shaderProgram);

    // Bind vertex data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBindVertexArray(m_vao);

    glBindTexture(GL_TEXTURE_1D, m_texture);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_1D, 0);

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

void SliceViewGL::updateTexture() {
    if (!is_opengl_initialized) {
        return;
    }

    setupTexture();
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

void SliceViewGL::setupVertexShader() {
    const std::string vertexShaderSource = VDS::ShaderGenerator::getVertexShaderCodeSlice2D();
    const GLchar* const shaderGLSL = vertexShaderSource.c_str();

    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &shaderGLSL, NULL);
    glCompileShader(m_vertexShader);
}

void SliceViewGL::setupFragmentShader() {
    const std::string fragmenntShaderSource = VDS::ShaderGenerator::getFragmentShaderCodeSlice2D(m_settings);
    const GLchar* const shaderGLSL = fragmenntShaderSource.c_str();

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &shaderGLSL, NULL);
    glCompileShader(m_fragmentShader);
}

void SliceViewGL::setupShaderProgram() {
    m_shaderProgram = glCreateProgram();

    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);
}

void SliceViewGL::setupTexture() {
    glDeleteTextures(1, &m_texture);
    glGenTextures(1, &m_texture);

    glBindTexture(GL_TEXTURE_1D, m_texture);


    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexImage1D(GL_TEXTURE_1D, 0, GL_R16, static_cast<GLsizei>(scaledHistogramCopy.size()), 0,
    //             GL_RED, GL_UNSIGNED_SHORT, scaledHistogramCopy.data());

    // unbind
    glBindTexture(GL_TEXTURE_1D, 0);
}
