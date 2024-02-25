#pragma once

#include <QObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QMutex>

#include <vector>

#include "renderer/shader/shader_settings.h"

#include <VDTK/common/CommonDataTypes.h>

class SliceViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT

public:
    SliceViewGL(QWidget* parent);

public slots:
    void updateTexture(GLuint texture);
    void setAxis(VDTK::VolumeAxis axis);
    void setPosition(int position);
    void setSize(VDTK::VolumeSize size);
    void setSpacing(VDTK::VolumeSpacing spacing);

    void applyValueWindow(bool active);
    void setValueWindowMethod(int method);
    void updateValueWindowWidth(float windowWidth);
    void updateValueWindowCenter(float windowCenter);
    void updateValueWindowOffset(float windowOffset);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void enterEvent(QEnterEvent* ev) override;
    void leaveEvent(QEvent* ev) override;

signals:
    void enterEventSignaled();
    void leaveEventSignaled();

private:
    void setupBuffers();
    void setupVertexArray();
    bool setupVertexShader();
    bool setupFragmentShader();
    bool setupShaderProgram();
    
    void updateViewPortSize(float width, float heigth);
    void updateSpacing();

    bool generateShaderProgram();
    void updateShaderUniforms();

    bool checkShaderCompileStatus(GLuint shader);
    bool checkShaderProgramLinkStatus(GLuint shaderProgram);

    bool is_opengl_initialized;

    // global buffer handles
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    // global shader hanldes
    GLuint m_vertexShader;
    GLuint m_fragmentShader;
    GLuint m_shaderProgram;
    // global texture handles
    GLuint m_texture;
        
    VDS::Slice2DShaderSettings m_settings;
};