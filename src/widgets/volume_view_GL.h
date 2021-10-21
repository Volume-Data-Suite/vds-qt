#pragma once

#include "../renderer/raycast_renderer_gl.h"

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QPoint>

class VolumeViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {

    Q_OBJECT

public:
    VolumeViewGL(QWidget* parent);
    int getTextureSizeMaximum();

public slots:
    void updateVolumeData(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing,
                          const std::vector<uint16_t>& volumeData);
    void setRenderLoop(bool onlyRerenderOnChange);
    void setBoundingBoxRenderStatus(bool active);
    void setSampleStepLength(double stepLength);
    void setThreshold(double threshold);
    void setRecommendedSampleStepLength(int factor);
    void setRaycastMethod(int method);
    void applyValueWindow(bool active);
    void setValueWindowMethod(int method);
    void updateValueWindowWidth(float windowWidth);
    void updateValueWindowCenter(float windowCenter);
    void updateValueWindowOffset(float windowOffset);
    void recieveVertexShaderFromRenderer(const QString& vertexShaderSource);
    void recieveFragmentShaderFromRenderer(const QString& fragmentShaderSource);
    void recieveVertexShaderFromUI(const QString& vertexShaderSource);
    void recieveFragmentShaderFromUI(const QString& fragmentShaderSource);
    void resetViewMatrixAndUpdate();

protected:
    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

signals:
    void updateFrametime(float frameTime, float renderEverything, float volumeRendering);
    void updateSampleStepLength(double stepLength);
    void sendVertexShaderToUI(const QString& vertexShaderSource);
    void sendFragmentShaderToUI(const QString& fragmentShaderSource);

private:
    void logQSurfaceFormat() const;
    void logRenderDeviceInfo(const QString& title, GLenum name);

    void setProjectionMatrix(float aspectRatio);
    void resetViewMatrix();

    QVector3D getArcBallVector(QPoint p);

    float calculateFrameTime(std::chrono::time_point<std::chrono::high_resolution_clock> start,
                             std::chrono::time_point<std::chrono::high_resolution_clock> end) const;

    VDS::RayCastRenderer m_rayCastRenderer;

    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;

    // mouse drag rotation
    bool m_leftButtonPressed;
    QPoint m_prevPos;
    float m_rotationSpeed;

    bool m_renderloop;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrameTimePoint;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrameTimeGUIUpdate;

    // maxium texture size supported by the current GPU driver
    int m_maxiumTextureSize;
};