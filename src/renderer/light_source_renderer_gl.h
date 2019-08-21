#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <vector>

#include "light_source.h"

namespace VDS {

class LightSourceRenderer : public QObject, protected QOpenGLFunctions_4_3_Core {

public:
    LightSourceRenderer(const QMatrix4x4* const projectionMatrix,
                        const QMatrix4x4* const viewMatrix);
    ~LightSourceRenderer();

    void render();
    void setup();
    void applyMatrices();
    const QMatrix4x4 getModelMatrix() const;

    void rotate(float angle, float x, float y, float z);
    void translate(float x, float y, float z);
    void scale(float factor);
    void resetModelMatrix();

	void addLightSource(const LightSource& lightSource);
	// delete by value. This eleminates a lot of synching between the GUI and this class
    void deleteLightSource(const LightSource& lightSource);
    
private:
    void setupBuffers();
    void setupVertexArray();
    void setupVertexShader();
    void setupFragmentShader();
    void setupShaderProgram();

    // global buffer handles
    GLuint m_vao_cube_vertices;
    GLuint m_vbo_cube_vertices;
    GLuint m_ibo_cube_elements;
    // global shader hanldes
    GLuint m_vertexShader;
    GLuint m_fragmentShader;
    GLuint m_shaderProgram;

    // Matrices
    const QMatrix4x4* const m_projectionMatrix;
    const QMatrix4x4* const m_viewMatrix;
    QMatrix4x4 m_rotationMatrix;
    QMatrix4x4 m_translationMatrix;
    QMatrix4x4 m_scaleMatrix;

	std::vector<LightSource> m_lightSources;
};
} // namespace VDS