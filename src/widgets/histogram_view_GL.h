#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <QObject>
#include <QMatrix4x4>

#include <vector>

class HistogramViewGL : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
	Q_OBJECT

public:
	HistogramViewGL(QWidget *parent);

	void updateHistogramData(const std::vector<uint16_t>& histo);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

private:
	void calculateScaledHistogram();

	void setupBuffers();
	void setupVertexArray();
	void setupVertexShader();
	void setupFragmentShader();
	void setupShaderProgram();

	std::vector<uint16_t> m_histogramData;
	std::vector<uint16_t> m_histogramDataScaled;

	uint16_t m_max;

	int m_width;
	int m_height;


	// global buffer handles
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ibo;
	// global shader hanldes
	GLuint m_vertexShader;
	GLuint m_fragmentShader;
	GLuint m_shaderProgram;

	// Matrices
	QMatrix4x4 m_projectionMatrix;
	QMatrix4x4 m_viewMatrix;
	QMatrix4x4 m_modelMatrix;
};