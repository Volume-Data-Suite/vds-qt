#pragma once

#include "../renderer/raycast_renderer_gl.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QPoint>

class VolumeViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
public:
	VolumeViewGL(QWidget *parent);

	void updateVolumeData(const std::array<uint32_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData);

protected:
	void initializeGL() override;

	void resizeGL(int w, int h) override;

	void paintGL() override;

	void mousePressEvent(QMouseEvent * e) override;
	void mouseReleaseEvent(QMouseEvent * e) override;
	void mouseMoveEvent(QMouseEvent * e) override;
	void wheelEvent(QWheelEvent * e) override;

private:
	void render();
	void logQSurfaceFormat() const;

	void setProjectionMatrix();
	void setViewMatrix();

	VDS::RayCastRenderer m_rayCastRenderer;

	QMatrix4x4 m_projectionMatrix;
	QMatrix4x4 m_viewMatrix;

	// mouse drag rotation
	bool m_leftButtonPressed;
	QPoint m_prevPos;
};