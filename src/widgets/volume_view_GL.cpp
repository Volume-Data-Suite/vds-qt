#include "volume_view_GL.h"

#include <QDebug>
#include <QMetaEnum>


VolumeViewGL::VolumeViewGL(QWidget *parent) : QOpenGLWidget(parent), m_rayCastRenderer(&m_projectionMatrix, &m_viewMatrix)
{
	setProjectionMatrix();
	setViewMatrix();

	m_leftButtonPressed = false;
	m_prevPos = {};
}

void VolumeViewGL::initializeGL()
{
	initializeOpenGLFunctions();

	logQSurfaceFormat();


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	// Enable blending so you can see different alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glCullFace(GL_FRONT);

	glClearDepth(1.0f);
	// Change the reference of the GL_COLOR_BUFFER_BIT
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	m_rayCastRenderer.setup();
	m_rayCastRenderer.translate(0.0f, 0.0f, -5.0f);
}

void VolumeViewGL::resizeGL(int w, int h)
{
	setProjectionMatrix();
	m_rayCastRenderer.applyMatrices();
}

void VolumeViewGL::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_rayCastRenderer.render();
}

void VolumeViewGL::mousePressEvent(QMouseEvent * e)
{
	if (e->button() == Qt::LeftButton)
	{
		m_leftButtonPressed = true;
		m_prevPos = e->pos();
		e->accept();
	}
}

void VolumeViewGL::mouseReleaseEvent(QMouseEvent * e)
{
	m_leftButtonPressed = false;
	m_prevPos = {};
	e->accept();
}

void VolumeViewGL::mouseMoveEvent(QMouseEvent * e)
{	
	if (!m_leftButtonPressed) {
		return;
	}

	QVector2D direction = QVector2D(e->pos()) - QVector2D(m_prevPos);

	// we want to rotate around the opposite axis of the mouse movement
	m_rayCastRenderer.rotate(direction.y(), direction.x());
	
	m_prevPos = e->pos();

	e->accept();

	this->update();
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
	constexpr QVector3D eye(0.0, 0.0, 2.0);
	// At which point should it look
	constexpr QVector3D lookAt(0.0, 0.0, -1.0);
	// Wich direction is up
	constexpr QVector3D up(0.0, 1.0, 0.0);

	m_viewMatrix.setToIdentity();
	m_viewMatrix.lookAt(eye, lookAt, up);
}
