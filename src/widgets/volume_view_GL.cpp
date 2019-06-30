#include "volume_view_GL.h"

#include <QDebug>
#include <QMetaEnum>

#include <chrono>

VolumeViewGL::VolumeViewGL(QWidget *parent) : QOpenGLWidget(parent), m_rayCastRenderer(&m_projectionMatrix, &m_viewMatrix)
{
	setProjectionMatrix(1.0f);
	setViewMatrix();

	m_leftButtonPressed = false;
	m_prevPos = {};

	m_renderloop = false;

	m_lastFrameTimePoint = std::chrono::high_resolution_clock::now();

	m_rotationSpeed = 200.0f;
}

void VolumeViewGL::updateVolumeData(const std::array<uint32_t, 3> size, const std::array<float, 3> spacing, const std::vector<uint16_t>& volumeData)
{
	m_rayCastRenderer.updateVolumeData(size, spacing, volumeData);
	
	this->update();
}

void VolumeViewGL::setRenderLoop(bool onlyRerenderOnChange)
{
	m_renderloop = !onlyRerenderOnChange;
	if (m_renderloop)
	{
		update();
	}
	else
	{
		emit updateFrametime(0.0f, 0.0f, 0.0f);
	}
}

void VolumeViewGL::initializeGL()
{
	initializeOpenGLFunctions();

	logQSurfaceFormat();
	logRenderDeviceInfo(QString("GPU Vendor"), GL_VENDOR);
	logRenderDeviceInfo(QString("OpenGL Renderer"), GL_RENDERER);
	logRenderDeviceInfo(QString("OpenGL Version"), GL_VERSION);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	// Enable blending so you can see different alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	// so we can go inside the volume and dont have to accumulated raycasts (because we do a single pass raycasting)
	glCullFace(GL_FRONT); 
	glFrontFace(GL_CCW);

	glClearDepth(1.0f);
	// Change the reference of the GL_COLOR_BUFFER_BIT
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	m_rayCastRenderer.setup();
	m_viewMatrix.translate(0.0f, 0.0f, -0.2f);
	
}

void VolumeViewGL::resizeGL(int w, int h)
{
	const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

	setProjectionMatrix(aspectRatio);
	m_rayCastRenderer.applyMatrices();
	m_rayCastRenderer.updateAspectRation(aspectRatio);
	m_rayCastRenderer.updateViewPortSize(w, h);
}

void VolumeViewGL::paintGL()
{
	const auto startRender = std::chrono::high_resolution_clock::now();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto startRenderVolume = std::chrono::high_resolution_clock::now();
	m_rayCastRenderer.render();
	const auto endRenderVolume = std::chrono::high_resolution_clock::now();

	const auto endRender = std::chrono::high_resolution_clock::now();
	
	const float renderTime = calculateFrameTime(startRender, endRender);
	const float renderTimeVolume = calculateFrameTime(startRenderVolume, endRenderVolume);
	float frameTime = calculateFrameTime(m_lastFrameTimePoint, endRender);

	m_lastFrameTimePoint = endRender;

	// dont spam with events for fps counter updates
	if (calculateFrameTime(m_lastFrameTimeGUIUpdate, endRender) >= 200.0f)
	{
		if (!m_renderloop)
		{
			// do not show FPS, since it would confuse users
			frameTime = 0.0f;
		}
		emit updateFrametime(frameTime, renderTime, renderTimeVolume);
		m_lastFrameTimeGUIUpdate = endRender;
	}
		
	if (m_renderloop)
	{
		update();
	}
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

	if (!m_renderloop)
	{
		emit updateFrametime(0.0f, 0.0f, 0.0f);
	}

	e->accept();
}

void VolumeViewGL::mouseMoveEvent(QMouseEvent * e)
{	
	if (!m_leftButtonPressed) {
		return;
	}

	const QVector3D oldPosition = getArcBallVector(m_prevPos);
	const QVector3D newPosition = getArcBallVector(e->pos());

	const float dot = QVector3D::dotProduct(oldPosition, newPosition);
	const float rotationAngle = std::acos(std::min(1.0f, dot)) * m_rotationSpeed;

	const QVector3D axisInCameraCoordinates = QVector3D::crossProduct(oldPosition, newPosition);
	const QVector3D axisInObjectCoordinates = m_rayCastRenderer.getModelMatrix().inverted() * axisInCameraCoordinates;

	m_rayCastRenderer.rotate(rotationAngle, axisInObjectCoordinates.x(), axisInObjectCoordinates.y(), axisInObjectCoordinates.z());

		
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
	m_viewMatrix.translate(0, 0, translateAmount);
	m_rayCastRenderer.applyMatrices();

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

void VolumeViewGL::logRenderDeviceInfo(const QString& title, GLenum name)
{
	const GLubyte* info = glGetString(name);

	//glGetString returns a null pointer on error.
	if (info)
	{
		//cast from const unsigned char* to const char*
		const QString string(reinterpret_cast<const char*>(info));
		qDebug().noquote() << title << ": " << string;
	}
}

void VolumeViewGL::setProjectionMatrix(float aspectRatio)
{
	constexpr GLfloat nearPlane = 0.0f;
	constexpr GLfloat farPlane = 10.0f;
	constexpr GLfloat verticalAngle = 90.0f;

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

QVector3D VolumeViewGL::getArcBallVector(QPoint p)
{
	QVector3D arcBallVector(
		1.0f * p.x() / this->width() * 2.0f - 1.0f,
		1.0f * p.y() / this->height() * 2.0f - 1.0f,
		0.0f);

	arcBallVector.setY(-arcBallVector.y());

	const float OPSquared = arcBallVector.x() * arcBallVector.x() + arcBallVector.y() * arcBallVector.y();

	if (OPSquared <= 1.0f)
	{
		// Pythagoras
		arcBallVector.setZ(std::sqrt(1 - OPSquared));
	}
	else
	{
		// nearest point
		arcBallVector.normalize();
	}

	return arcBallVector;
}

float VolumeViewGL::calculateFrameTime(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) const
{
	const std::chrono::duration<float> duration = end - start;
	const std::chrono::nanoseconds nanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

	return static_cast<float>(nanoSeconds.count()) / 1000000.0;
}
