#include "histogram_view_GL.h"

#include <algorithm>
#include <QtConcurrent>
#include <QFuture>

HistogramViewGL::HistogramViewGL(QWidget* parent)
    : QOpenGLWidget(parent), m_histogramData(UINT16_MAX + 1, 0), m_histogramDataScaled(10, 0) {
    m_width = m_height = 1;

    connect(this, &HistogramViewGL::updateHistogram, this, &HistogramViewGL::updateTexture);
}

void HistogramViewGL::updateHistogramData(const std::vector<uint16_t>& histo, bool ignoreBorders) {
    setHistogramData(histo);

    if (ignoreBorders) {
        QMutexLocker locker(&m_mutexHistogram);
        m_histogramData[0] = 0;
        m_histogramData[UINT16_MAX] = 0;
    }

    calculateScaledHistogram();
    update();
}

void HistogramViewGL::initializeGL() {
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
    setupTexture();
}

void HistogramViewGL::resizeGL(int w, int h) {
    setWidth(w > 1 ? w : 1);
    setHeight(h > 1 ? h : 1);
    glViewport(0, 0, getWidth(), getHeight());
    calculateScaledHistogram();

    glUseProgram(m_shaderProgram);
    const GLuint viewport = glGetUniformLocation(m_shaderProgram, "viewport");
    glUniform2f(viewport, static_cast<float>(getWidth()), static_cast<float>(getHeight()));
    glUseProgram(0);
}

void HistogramViewGL::paintGL() {
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

const std::vector<uint16_t> HistogramViewGL::getHistogramData() {
    QMutexLocker locker(&m_mutexHistogram);
    return m_histogramData;
}

void HistogramViewGL::setHistogramData(const std::vector<uint16_t>& histogram) {
    QMutexLocker locker(&m_mutexHistogram);
    m_histogramData = histogram;
}

const std::vector<uint16_t> HistogramViewGL::getScaledHistogramData() {
    QMutexLocker locker(&m_mutexScaledHistogram);
    return m_histogramDataScaled;
}

void HistogramViewGL::setScaledHistogramData(const std::vector<uint16_t>& histogram) {
    QMutexLocker locker(&m_mutexScaledHistogram);
    m_histogramDataScaled = histogram;
}

int HistogramViewGL::getWidth() {
    QMutexLocker locker(&m_mutexWidth);
    return m_width;
}

void HistogramViewGL::setWidth(int width) {
    QMutexLocker locker(&m_mutexWidth);
    m_width = width;
}

int HistogramViewGL::getHeight() {
    QMutexLocker locker(&m_mutexHeight);
    return m_height;
}

void HistogramViewGL::setHeight(int height) {
    QMutexLocker locker(&m_mutexHeight);
    m_height = height;
}

uint16_t HistogramViewGL::getMax() {
    QMutexLocker locker(&m_mutexMax);
    return m_max;
}

void HistogramViewGL::setMax(uint16_t max) {
    QMutexLocker locker(&m_mutexMax);
    m_max = max;
}

void HistogramViewGL::calculateScaledHistogram() {
    QFuture<void> future = QtConcurrent::run([&]() {
        QThread::currentThread()->setObjectName("Compute Scaled Histogram Thread");

        const int width = getWidth();
        std::vector<uint16_t> histogramScaled(width);
        const std::vector<uint16_t> histogram = getHistogramData();

        const std::size_t size = histogram.size();
        const std::size_t sectionSize = size / width;

        for (std::size_t index = 0; index < width; index++) {
            histogramScaled[index] =
                *std::max_element(histogram.begin() + index * sectionSize,
                                  histogram.begin() + (index + 1) * sectionSize - 1);
        }

        setMax(*std::max_element(histogramScaled.begin(), histogramScaled.end()));
        setScaledHistogramData(histogramScaled);

        emit(updateHistogram());

        return;
    });
}

void HistogramViewGL::updateTexture() {
    setupTexture();

    glUseProgram(m_shaderProgram);
    const float maxNormalized = static_cast<float>(getMax()) / static_cast<float>(UINT16_MAX);
    const GLuint max = glGetUniformLocation(m_shaderProgram, "max");
    glUniform1f(max, maxNormalized);
    glUseProgram(0);

    update();
}

void HistogramViewGL::setupBuffers() {
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

void HistogramViewGL::setupVertexArray() {
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

void HistogramViewGL::setupVertexShader() {
    const GLchar* const shaderGLSL = "#version 430 core \n"

                                     "in vec3 inPos; \n"

                                     "void main() \n"
                                     "{ \n"
                                     "	gl_Position = vec4(inPos.x, inPos.y, inPos.z, 1.0f); \n"
                                     "} \n";

    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &shaderGLSL, NULL);
    glCompileShader(m_vertexShader);
}

void HistogramViewGL::setupFragmentShader() {
    const GLchar* const shaderGLSL =
        "#version 430 core \n"

        "uniform vec2 viewport; \n"
        "uniform float max; \n"

        "uniform sampler1D histogram; \n"

        "out vec4 FragColor; \n"

        "void main() \n"
        "{ \n"
        "	const float value = texture(histogram, gl_FragCoord.x / viewport.x).r / max; \n"
        "	const float correctValue = value >= gl_FragCoord.y / viewport.y ? 1.0f : 0.0f; \n"
        "	FragColor = vec4(vec3(correctValue), 1.0f); \n"
        "} \n";

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &shaderGLSL, NULL);
    glCompileShader(m_fragmentShader);
}

void HistogramViewGL::setupShaderProgram() {
    m_shaderProgram = glCreateProgram();

    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);
}

void HistogramViewGL::setupTexture() {
    glDeleteTextures(1, &m_texture);
    glGenTextures(1, &m_texture);

    glBindTexture(GL_TEXTURE_1D, m_texture);

    // to make it all thread safe so the size and data amount match
    const auto scaledHistogramCopy = getScaledHistogramData();

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R16, scaledHistogramCopy.size(), 0, GL_RED, GL_UNSIGNED_SHORT,
                 scaledHistogramCopy.data());

    // unbind
    glBindTexture(GL_TEXTURE_1D, 0);
}
