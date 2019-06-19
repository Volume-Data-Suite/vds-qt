#include "volume_view_GL.h"

#include <QDebug>
#include <QMetaEnum>

#include <chrono>


VolumeViewGL::VolumeViewGL(QWidget *parent) : QOpenGLWidget(parent), m_rayCastRenderer(&m_projectionMatrix, &m_viewMatrix)
{
	setProjectionMatrix();
	setViewMatrix();

	m_leftButtonPressed = false;
	m_prevPos = {};
}

void VolumeViewGL::updateVolumeData(const std::array<uint32_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData)
{
	m_rayCastRenderer.updateVolumeData(size, spacing, volumeData);

	m_rayCastRenderer.resetModelMatrix();

	this->update();
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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); 
	glFrontFace(GL_CCW);

	glClearDepth(1.0f);
	// Change the reference of the GL_COLOR_BUFFER_BIT
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	m_rayCastRenderer.setup();
	m_viewMatrix.translate(0.0f, 0.0f, -5.0f);
}

void VolumeViewGL::resizeGL(int w, int h)
{
	setProjectionMatrix();
	m_rayCastRenderer.applyMatrices();
}

void VolumeViewGL::paintGL()
{
	const auto start = std::chrono::high_resolution_clock::now();

	render();

	const auto end = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double> duration = end - start;
	const std::chrono::nanoseconds nanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

	const double milliSeconds = static_cast<double>(nanoSeconds.count()) / 1000000.0;

	qDebug() << "Milli Seconds to render: " << milliSeconds << "which equals to " << 1000.0 / milliSeconds << " FPS";
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

	// does not cause an immediate repaint; instead it schedules a paint event for processing 
	// when Qt returns to the main event loop. This permits Qt to optimize for more speed and
	// less flicker than a call to repaint() does.
	this->update();
}

void VolumeViewGL::wheelEvent(QWheelEvent * e)
{
	const float translateAmount = static_cast<float>(e->delta()) / 150.0f;
	m_viewMatrix.translate(0.0f, 0.0f, translateAmount);

	m_rayCastRenderer.applyMatrices();

	e->accept();
	this->update();
}


void VolumeViewGL::render()
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
	constexpr QVector3D eye(0.0, 0.0, 2.0);
	// At which point should it look
	constexpr QVector3D lookAt(0.0, 0.0, -1.0);
	// Wich direction is up
	constexpr QVector3D up(0.0, 1.0, 0.0);

	m_viewMatrix.setToIdentity();
	m_viewMatrix.lookAt(eye, lookAt, up);
}
