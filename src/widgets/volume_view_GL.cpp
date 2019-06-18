#include "volume_view_GL.h"

#include <QDebug>
#include <QMetaEnum>


VolumeViewGL::VolumeViewGL(QWidget *parent) : QOpenGLWidget(parent), m_rayCastRenderer(&m_projectionMatrix, &m_viewMatrix)
{
	setProjectionMatrix();
	setViewMatrix();
}

void VolumeViewGL::initializeGL()
{
	initializeOpenGLFunctions();

	logQSurfaceFormat();


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


	// Enable blending so you can see different alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glCullFace(GL_FRONT_AND_BACK);

	glClearDepth(1.0f);
	// Change the reference of the GL_COLOR_BUFFER_BIT
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	m_rayCastRenderer.setup();
}

void VolumeViewGL::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);

	setProjectionMatrix();
	m_rayCastRenderer.applyMatrices();
}

void VolumeViewGL::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	m_rayCastRenderer.render();
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

void VolumeViewGL::setProjectionMatrix()
{
	const GLfloat aspectRatio = (GLfloat)this->width() / (GLfloat)this->height();
	constexpr GLfloat nearPlane = 0.0f;
	constexpr GLfloat farPlane = 10.0f;
	constexpr GLfloat verticalAngle = 45.0f;

	m_projectionMatrix.setToIdentity();
	m_projectionMatrix.perspective(verticalAngle, aspectRatio, nearPlane, farPlane);
}
void VolumeViewGL::setViewMatrix()
{
	// Where is the camera
	constexpr QVector3D eye(0.0, 0.0, 0.0);
	// At which point should it look
	constexpr QVector3D lookAt(0.0, 0.0, -1.0);
	// Wich direction is up
	constexpr QVector3D up(0.0, 1.0, 0.0);

	m_viewMatrix.setToIdentity();
	m_viewMatrix.lookAt(eye, lookAt, up);
}
