#pragma once

#include <QObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QMutex>

#include <vector>

class SliceViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT

public:
    SliceViewGL(QWidget* parent);

public slots:
    void updateTexture();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void enterEvent(QEvent* ev) override;
    void leaveEvent(QEvent* ev) override;

signals:
    void updateHistogram();
    void enterEventSignaled();
    void leaveEventSignaled();

private:

    void setupBuffers();
    void setupVertexArray();
    void setupVertexShader();
    void setupFragmentShader();
    void setupShaderProgram();
    void setupTexture();

    bool is_opengl_initialized;

    // global buffer handles
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    // global shader hanldes
    GLuint m_vertexShader;
    GLuint m_fragmentShader;
    GLuint m_shaderProgram;
    // globatl texture handles
    GLuint m_texture;
};