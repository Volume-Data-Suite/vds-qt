#pragma once

#include <QObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QMutex>

#include <vector>

class HistogramViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT

public:
    HistogramViewGL(QWidget* parent);

public slots:
    // ignoreBorders if active, 0 and Max (UINT16MAX) get ingored, since they are crowed by linear
    // windowing
    void updateHistogramData(const std::vector<uint16_t>& histo, bool ignoreBorders);
    void updateTexture();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    const std::vector<uint16_t> getHistogramData();
    void setHistogramData(const std::vector<uint16_t>& histogram);
    const std::vector<uint16_t> getScaledHistogramData();
    void setScaledHistogramData(const std::vector<uint16_t>& histogram);

    int getWidth();
    void setWidth(int width);
    int getHeight();
    void setHeight(int height);

    uint16_t getMax();
    void setMax(uint16_t max);

signals:
    void updateHistogram();

private:
    void calculateScaledHistogram();

    void setupBuffers();
    void setupVertexArray();
    void setupVertexShader();
    void setupFragmentShader();
    void setupShaderProgram();
    void setupTexture();

    std::vector<uint16_t> m_histogramData;
    std::vector<uint16_t> m_histogramDataScaled;

    bool is_opengl_initialized;

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

    // Threading variables
    mutable QMutex m_mutexHistogram;
    mutable QMutex m_mutexScaledHistogram;
    mutable QMutex m_mutexWidth;
    mutable QMutex m_mutexHeight;
    mutable QMutex m_mutexMax;
};