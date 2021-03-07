#pragma once

#include "../renderer/light_source.h"
#include "../renderer/light_source_renderer_gl.h"
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

    void updateVolumeData(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing,
                          const std::vector<uint16_t>& volumeData);

public slots:
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
    void addLightSource(const VDS::LightSource& lightSource);
    // delete by value. This eleminates a lot of synching between the GUI and this class
    void deleteLightSource(const VDS::LightSource& lightSource);
    void rotateVolumeOnly();
    void rotateVolumeAndLights();
    void rotateLightsOnly();

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
    void updateLightSourceValues();
    void updateLightSourceCount();

private:
    void logQSurfaceFormat() const;
    void logRenderDeviceInfo(const QString& title, GLenum name);

    void setProjectionMatrix(float aspectRatio);
    void resetViewMatrix();
    void setupLightSources();

    QVector3D getArcBallVector(QPoint p);

    float calculateFrameTime(std::chrono::steady_clock::time_point start,
                             std::chrono::steady_clock::time_point end) const;

    VDS::RayCastRenderer m_rayCastRenderer;
    VDS::LightSourceRenderer m_lightSourceRenderer;
    std::vector<VDS::LightSource> m_lightSources;

    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;

    // mouse drag rotation
    bool m_leftButtonPressed;
    QPoint m_prevPos;
    float m_rotationSpeed;
    bool m_rotateVolume;
    bool m_rotateLights;

    bool m_renderloop;
    std::chrono::steady_clock::time_point m_lastFrameTimePoint;
    std::chrono::steady_clock::time_point m_lastFrameTimeGUIUpdate;
};