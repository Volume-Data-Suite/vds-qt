#pragma once

#include "../renderer/raycast_renderer_gl.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QPoint>
#include <QObject>


class VolumeViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{

	Q_OBJECT

public:
	VolumeViewGL(QWidget *parent);

	void updateVolumeData(const std::array<uint32_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData);

public slots:
	void setRenderLoop(bool onlyRerenderOnChange);

protected:
	void initializeGL() override;

	void resizeGL(int w, int h) override;

	void paintGL() override;

	void mousePressEvent(QMouseEvent * e) override;
	void mouseReleaseEvent(QMouseEvent * e) override;
	void mouseMoveEvent(QMouseEvent * e) override;
	void wheelEvent(QWheelEvent * e) override;

signals:
	void updateFrametime(float frameTime, float renderEverything, float volumeRendering);

private:
	void logQSurfaceFormat() const;
	void logRenderDeviceInfo(const QString& title, GLenum name);

	void setProjectionMatrix();
	void setViewMatrix();

	float calculateFrameTime(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) const;

	VDS::RayCastRenderer m_rayCastRenderer;

	QMatrix4x4 m_projectionMatrix;
	QMatrix4x4 m_viewMatrix;

	// mouse drag rotation
	bool m_leftButtonPressed;
	QPoint m_prevPos;

	bool m_renderloop;
	std::chrono::steady_clock::time_point m_lastFrameTimePoint;
	std::chrono::steady_clock::time_point m_lastFrameTimeGUIUpdate;
};