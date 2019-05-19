#pragma once


#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>

class VolumeViewGL : public QOpenGLWidget, QOpenGLFunctions_4_3_Core
{
public:
	VolumeViewGL(QWidget *parent);

protected:
	void initializeGL() override;

	void resizeGL(int w, int h);

	void paintGL() override;


private:
	void logQSurfaceFormat() const;
};