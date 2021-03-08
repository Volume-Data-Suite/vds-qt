#pragma once

#include <QObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>

#include <vector>

class HistogramViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT

public:
    HistogramViewGL(QWidget* parent);

    // ignoreBorders if active, 0 and Max (UINT16MAX) get ingored, since they are crowed by linear
    // windowing
    void updateHistogramData(const std::vector<uint16_t>& histo, bool ignoreBorders);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void calculateScaledHistogram();
    void updateTexture();

    void setupBuffers();
    void setupVertexArray();
    void setupVertexShader();
    void setupFragmentShader();
    void setupShaderProgram();
    void setupTexture();

    std::vector<uint16_t> m_histogramData;
    std::vector<uint16_t> m_histogramDataScaled;

    uint16_t m_max;

    int m_width;
    int m_height;

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