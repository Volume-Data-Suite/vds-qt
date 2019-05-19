#include "volume_view_GL.h"

#include <QDebug>
#include <QMetaEnum>

VolumeViewGL::VolumeViewGL(QWidget *parent) : QOpenGLWidget(parent) { }

void VolumeViewGL::initializeGL()
{
	initializeOpenGLFunctions();

	logQSurfaceFormat();


	// Enable blending so you can see different alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearDepth(1.0f);
	// Change the reference of the GL_COLOR_BUFFER_BIT
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void VolumeViewGL::resizeGL(int w, int h)
{
}

void VolumeViewGL::paintGL()
{
}

void VolumeViewGL::logQSurfaceFormat() const
{
	const QSurfaceFormat fmt = this->format();
	qDebug().nospace() << "OpenGL " << fmt.majorVersion() << "." << fmt.minorVersion();
	qDebug().noquote() << "Profile:"
		<< QMetaEnum::fromType<QSurfaceFormat::OpenGLContextProfile>().valueToKey(fmt.profile());
	qDebug().noquote() << "Options:" << QMetaEnum::fromType<QSurfaceFormat::FormatOption>().valueToKeys(fmt.options());
	qDebug().noquote() << "Renderable Type:"
		<< QMetaEnum::fromType<QSurfaceFormat::RenderableType>().valueToKey(fmt.renderableType());
	qDebug().noquote() << "Swap Behavior:"
		<< QMetaEnum::fromType<QSurfaceFormat::SwapBehavior>().valueToKey(fmt.swapBehavior());
	qDebug() << "Swap Interval:" << fmt.swapInterval();
}
