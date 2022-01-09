#include "volume_view_GL.h"

#include <QDebug>
#include <QMetaEnum>

#include <chrono>
#include <cmath>

//  dedicated video memory, total size (in kb) of the GPU memory
#define GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX 0x9047
// total available memory, total size (in Kb) of the memory available for allocations
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
// current available dedicated video memory (in kb), currently unused GPU memory
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
// count of total evictions seen by system
#define GPU_MEMORY_INFO_EVICTION_COUNT_NVX 0x904A
// size of total video memory evicted (in kb)
#define GPU_MEMORY_INFO_EVICTED_MEMORY_NVX 0x904B

VolumeViewGL::VolumeViewGL(QWidget* parent)
    : QOpenGLWidget(parent), m_rayCastRenderer(&m_projectionMatrix, &m_viewMatrix) {
    setProjectionMatrix(1.0f);
    resetViewMatrix();

    m_leftButtonPressed = false;
    m_prevPos = {};

    m_renderloop = false;

    m_lastFrameTimePoint = std::chrono::high_resolution_clock::now();

    m_rotationSpeed = 200.0f;

    // connect Shader Debug Editor
    connect(&m_rayCastRenderer, &VDS::RayCastRenderer::provideGeneratedVertexShader, this,
            &VolumeViewGL::recieveVertexShaderFromRenderer);
    connect(&m_rayCastRenderer, &VDS::RayCastRenderer::provideGeneratedFragmentShader, this,
            &VolumeViewGL::recieveFragmentShaderFromRenderer);
}

void VolumeViewGL::updateVolumeData(const std::array<std::size_t, 3> size,
                                    const std::array<float, 3> spacing,
                                    const std::vector<uint16_t>& volumeData) {
    m_rayCastRenderer.updateVolumeData(size, spacing, volumeData);

    // set sample step length to 1x optimal samples per ray
    setRecommendedSampleStepLength(0);

    resetViewMatrix();
    m_rayCastRenderer.applyMatrices();

    this->update();
}

int VolumeViewGL::getTextureSizeMaximum() {
    return m_maxiumTextureSize;
}

GLuint VolumeViewGL::getTextureHandle() const {
    return m_rayCastRenderer.getTextureHandle();
}

void VolumeViewGL::setRenderLoop(bool onlyRerenderOnChange) {
    m_renderloop = !onlyRerenderOnChange;
    if (m_renderloop) {
        update();
    } else {
        emit updateFrametime(0.0f, 0.0f, 0.0f);
    }
}

void VolumeViewGL::setBoundingBoxRenderStatus(bool active) {
    m_rayCastRenderer.setBoundingBoxRenderStatus(active);
    update();
}

void VolumeViewGL::setRenderSliceBorders(bool active) {
    m_rayCastRenderer.setRenderSliceBorders(active);
    update();
}

void VolumeViewGL::setSampleStepLength(double stepLength) {
    m_rayCastRenderer.updateSampleStepLength(static_cast<float>(stepLength));
    update();
}

void VolumeViewGL::setThreshold(double threshold) {
    m_rayCastRenderer.updateThreshold(threshold);
    update();
}

void VolumeViewGL::setRecommendedSampleStepLength(int factor) {
    // factor is the drop down menu index (0 --> 1x, 1 --> 2x, 2 --> 3x, ...)
    const float stepLenth =
        m_rayCastRenderer.getMinimalSampleStepLength() / static_cast<float>(factor + 1);
    emit updateSampleStepLength(static_cast<double>(stepLenth));
}

void VolumeViewGL::setRaycastMethod(int method) {
    m_rayCastRenderer.setRayCastMethod(method);
    update();
}

void VolumeViewGL::applyValueWindow(bool active) {
    m_rayCastRenderer.applyValueWindow(active);
    update();
}

void VolumeViewGL::setValueWindowMethod(int method) {
    m_rayCastRenderer.setValueWindowMethod(method);
    update();
}

void VolumeViewGL::updateValueWindowWidth(float windowWidth) {
    m_rayCastRenderer.updateValueWindowWidth(1.0f / static_cast<float>(UINT16_MAX) * windowWidth);
    update();
}

void VolumeViewGL::updateValueWindowCenter(float windowCenter) {
    m_rayCastRenderer.updateValueWindowCenter(1.0f / static_cast<float>(UINT16_MAX) * windowCenter);
    update();
}

void VolumeViewGL::updateValueWindowOffset(float windowOffset) {
    m_rayCastRenderer.updateValueWindowOffset(1.0f / static_cast<float>(UINT16_MAX) * windowOffset);
    update();
}

void VolumeViewGL::recieveVertexShaderFromRenderer(const QString& vertexShaderSource) {
    sendVertexShaderToUI(vertexShaderSource);
}

void VolumeViewGL::recieveFragmentShaderFromRenderer(const QString& fragmentShaderSource) {
    sendFragmentShaderToUI(fragmentShaderSource);
}

void VolumeViewGL::recieveVertexShaderFromUI(const QString& vertexShaderSource) {
    m_rayCastRenderer.overwriteVertexShaderRayCasting(vertexShaderSource);
    update();
}

void VolumeViewGL::recieveFragmentShaderFromUI(const QString& fragmentShaderSource) {
    m_rayCastRenderer.overwriteFragmentShaderRayCasting(fragmentShaderSource);
    update();
}

void VolumeViewGL::initializeGL() {
    initializeOpenGLFunctions();

    logQSurfaceFormat();
    logRenderDeviceInfo(QString("GPU Vendor"), GL_VENDOR);
    logRenderDeviceInfo(QString("OpenGL Renderer"), GL_RENDERER);
    logRenderDeviceInfo(QString("OpenGL Version"), GL_VERSION);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Enable blending so you can see different alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    // so we can go inside the volume and dont have to accumulated raycasts (because we do a single
    // pass raycasting)
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // glEnable(GL_MULTISAMPLE);

    glClearDepth(1.0f);
    // Change the reference of the GL_COLOR_BUFFER_BIT
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_rayCastRenderer.setup();

    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &m_maxiumTextureSize);
}

void VolumeViewGL::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

    setProjectionMatrix(aspectRatio);
    m_rayCastRenderer.applyMatrices();
    m_rayCastRenderer.updateAspectRation(aspectRatio);
    m_rayCastRenderer.updateViewPortSize(static_cast<float>(w), static_cast<float>(h));
}

void VolumeViewGL::paintGL() {
    const auto startRender = std::chrono::high_resolution_clock::now();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto startRenderVolume = std::chrono::high_resolution_clock::now();
    m_rayCastRenderer.render();
    const auto endRenderVolume = std::chrono::high_resolution_clock::now();

    const auto endRender = std::chrono::high_resolution_clock::now();

    const float renderTime = calculateFrameTime(startRender, endRender);
    const float renderTimeVolume = calculateFrameTime(startRenderVolume, endRenderVolume);
    float frameTime = calculateFrameTime(m_lastFrameTimePoint, endRender);

    m_lastFrameTimePoint = endRender;

    // dont spam with events for fps counter updates
    if (calculateFrameTime(m_lastFrameTimeGUIUpdate, endRender) >= 200.0f) {
        if (!m_renderloop) {
            // do not show FPS, since it would confuse users
            frameTime = 0.0f;
        }
        emit updateFrametime(frameTime, renderTime, renderTimeVolume);
        m_lastFrameTimeGUIUpdate = endRender;
    }

#ifdef _DEBUG
    // check OpenGL error
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        qDebug() << "OpenGL error: " << err << endl;
    }
#endif // _DEBUG

    if (m_renderloop) {
        update();
    }
}

void VolumeViewGL::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_leftButtonPressed = true;
        m_prevPos = e->pos();
        e->accept();
    }
}

void VolumeViewGL::mouseReleaseEvent(QMouseEvent* e) {
    m_leftButtonPressed = false;
    m_prevPos = {};

    if (!m_renderloop) {
        emit updateFrametime(0.0f, 0.0f, 0.0f);
    }

    e->accept();
}

void VolumeViewGL::mouseMoveEvent(QMouseEvent* e) {
    if (!m_leftButtonPressed) {
        return;
    }

    const QVector3D oldPosition = getArcBallVector(m_prevPos);
    const QVector3D newPosition = getArcBallVector(e->pos());

    const float dot = QVector3D::dotProduct(oldPosition, newPosition);
    const float rotationAngle = std::acos(std::min(1.0f, dot)) * m_rotationSpeed;

    const QVector3D axisInCameraCoordinates = QVector3D::crossProduct(oldPosition, newPosition);

    const QVector3D axisInObjectCoordinates =
        m_rayCastRenderer.getModelMatrix().inverted() * axisInCameraCoordinates;
    m_rayCastRenderer.rotate(rotationAngle, axisInObjectCoordinates.x(),
                             axisInObjectCoordinates.y(), axisInObjectCoordinates.z());

    m_prevPos = e->pos();

    e->accept();

    // does not cause an immediate repaint; instead it schedules a paint event for processing
    // when Qt returns to the main event loop. This permits Qt to optimize for more speed and
    // less flicker than a call to repaint() does.
    this->update();
}

void VolumeViewGL::wheelEvent(QWheelEvent* e) {
    const float translateAmount = static_cast<float>(e->angleDelta().y()) / 2500.0f;
    m_viewMatrix.translate(0, 0, translateAmount);
    m_rayCastRenderer.applyMatrices();

    e->accept();
    this->update();
}

void VolumeViewGL::logQSurfaceFormat() const {
    const QSurfaceFormat fmt = this->format();
    qDebug().nospace() << "OpenGL " << fmt.majorVersion() << "." << fmt.minorVersion();
    qDebug().noquote() << "Profile:"
                       << QMetaEnum::fromType<QSurfaceFormat::OpenGLContextProfile>().valueToKey(
                              fmt.profile());
    qDebug().noquote() << "Options:"
                       << QMetaEnum::fromType<QSurfaceFormat::FormatOption>().valueToKeys(
                              fmt.options());
    qDebug().noquote() << "Renderable Type:"
                       << QMetaEnum::fromType<QSurfaceFormat::RenderableType>().valueToKey(
                              fmt.renderableType());
    qDebug().noquote() << "Swap Behavior:"
                       << QMetaEnum::fromType<QSurfaceFormat::SwapBehavior>().valueToKey(
                              fmt.swapBehavior());
    qDebug() << "Swap Interval:" << fmt.swapInterval();
}

void VolumeViewGL::logRenderDeviceInfo(const QString& title, GLenum name) {
    const GLubyte* info = glGetString(name);

    // glGetString returns a null pointer on error.
    if (info) {
        // cast from const unsigned char* to const char*
        const QString string(reinterpret_cast<const char*>(info));
        qDebug().noquote() << title << ": " << string;
    }
}

void VolumeViewGL::setProjectionMatrix(float aspectRatio) {
    constexpr GLfloat nearPlane = 0.0001f;
    constexpr GLfloat farPlane = 10.0f;
    constexpr GLfloat verticalAngle = 90.0f;

    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(verticalAngle, aspectRatio, nearPlane, farPlane);
}
void VolumeViewGL::resetViewMatrix() {
    // Where is the camera
    constexpr QVector3D eye(0.0, 0.0, 2.0);
    // At which point should it look
    constexpr QVector3D lookAt(0.0, 0.0, -1.0);
    // Wich direction is up
    constexpr QVector3D up(0.0, 1.0, 0.0);

    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(eye, lookAt, up);
    m_viewMatrix.translate(0.0f, 0.0f, -0.2f);
}

bool VolumeViewGL::collectVRAMInfo(GLint& dedicatedMemory, GLint& totalAvailableMemory,
                                   GLint& availableDedicatedMemory, GLint& envictionCount,
                                   GLint& envictedMemory) {

    if (GL_NVX_gpu_memory_info) {
        glGetIntegerv(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &dedicatedMemory);
        glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalAvailableMemory);
        glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableDedicatedMemory);
        glGetIntegerv(GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &envictionCount);
        glGetIntegerv(GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &envictedMemory);
        return dedicatedMemory != 0;
    } else {
        dedicatedMemory = 0;
        totalAvailableMemory = 0;
        availableDedicatedMemory = 0;
        envictionCount = 0;
        envictedMemory = 0;
        return false;
    }
}

void VolumeViewGL::resetViewMatrixAndUpdate() {
    resetViewMatrix();
    m_rayCastRenderer.resetModelMatrix();
    update();
}

void VolumeViewGL::recieveVRAMinfoUpdateRequest() {

    int dedicatedMemory = 0;
    int totalAvailableMemory = 0;
    int availableDedicatedMemory = 0;
    int envictionCount = 0;
    int envictedMemory = 0;
    bool success = collectVRAMInfo(dedicatedMemory, totalAvailableMemory, availableDedicatedMemory,
                                   envictionCount, envictedMemory);

    emit sendVRAMinfoUpdate(success, dedicatedMemory, totalAvailableMemory,
                            availableDedicatedMemory,
                            envictionCount, envictedMemory);
}

void VolumeViewGL::setSliceXYPosition(float position) {
    m_rayCastRenderer.setSliceXYPosition(position);
    update();
}

void VolumeViewGL::setSliceXZPosition(float position) {
    m_rayCastRenderer.setSliceXZPosition(position);
    update();
}

void VolumeViewGL::setSliceYZPosition(float position) {
    m_rayCastRenderer.setSliceYZPosition(position);
    update();
}

QVector3D VolumeViewGL::getArcBallVector(QPoint p) {
    QVector3D arcBallVector(1.0f * p.x() / this->width() * 2.0f - 1.0f,
                            1.0f * p.y() / this->height() * 2.0f - 1.0f, 0.0f);

    arcBallVector.setY(-arcBallVector.y());

    const float OPSquared =
        arcBallVector.x() * arcBallVector.x() + arcBallVector.y() * arcBallVector.y();

    if (OPSquared <= 1.0f) {
        // Pythagoras
        arcBallVector.setZ(std::sqrt(1 - OPSquared));
    } else {
        // nearest point
        arcBallVector.normalize();
    }

    return arcBallVector;
}

float VolumeViewGL::calculateFrameTime(
    std::chrono::time_point<std::chrono::high_resolution_clock> start,
    std::chrono::time_point<std::chrono::high_resolution_clock> end) const {
    const std::chrono::duration<float> duration = end - start;
    const std::chrono::nanoseconds nanoSeconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

    return static_cast<float>(nanoSeconds.count()) / 1000000.0;
}
