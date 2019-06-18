#pragma once

#include "../renderer/raycast_renderer_gl.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <QMatrix4x4>

class VolumeViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
public:
	VolumeViewGL(QWidget *parent);

protected:
	void initializeGL() override;

	void resizeGL(int w, int h);

	void paintGL() override;
	

private:
	void logQSurfaceFormat() const;

	void setProjectionMatrix();
	void setViewMatrix();

	VDS::RayCastRenderer m_rayCastRenderer;

	QMatrix4x4 m_projectionMatrix;
	QMatrix4x4 m_viewMatrix;

};