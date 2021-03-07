#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <array>
#include "shader/shader_generator.h"
#include "textures/noise_texture_2D.h"
#include "textures/volume_data_3D_texture.h"

namespace VDS {

enum class RenderModes {
    Mesh,
    Borders,
};

class RayCastRenderer : public QObject, protected QOpenGLFunctions_4_3_Core {

public:
    RayCastRenderer(const QMatrix4x4* const projectionMatrix, const QMatrix4x4* const viewMatrix);
    ~RayCastRenderer();

    void render();

    bool setup();

    void applyMatrices();

    void rotate(float angle, float x, float y, float z);
    void translate(float x, float y, float z);
    void scale(float factor);
    void resetModelMatrix();

    void updateVolumeData(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing,
                          const std::vector<uint16_t>& volumeData);

    // TODO: Dont need a function for that. get the data from projection matrix on projection matrix
    // update
    void updateAspectRation(float ratio);

    void updateViewPortSize(float width, float heigth);

    void updateSampleStepLength(float stepLength);
    void updateThreshold(float threshold);

    void applyValueWindow(bool active);
    void setValueWindowMethod(int method);
    void updateValueWindowWidth(float windowWidth);
    void updateValueWindowCenter(float windowCenter);
    void updateValueWindowOffset(float windowOffset);

    void setRayCastMethod(int method);

    const std::array<float, 3> getPosition() const;
    const QMatrix4x4 getModelMatrix() const;
    // returns the sample step length which is required, if we do not want to skip any voxels
    const float getMinimalSampleStepLength() const;

    void setBoundingBoxColor(const std::array<float, 4>& color);

    void setBoundingBoxRenderStatus(bool active);

private:
    void renderVolume();
    void renderMesh();
    void renderVolumeBorders();

    void setupBuffers();
    void setupVertexArray(RenderModes renderMode);
    bool setupVertexShaderRayCasting();
    bool setupFragmentShaderRayCasting();
    bool setupShaderProgramRayCasting();

    bool generateRaycastShaderProgram();

    bool setupVertexShaderBoundingBox();
    bool setupFragmentShaderBoundingBox();
    bool setupShaderProgramBoundingBox();

    void setAxisAlignedBoundingBox(const std::array<float, 3>& extent);

    bool checkShaderCompileStatus(GLuint shader);
    bool checkShaderProgramLinkStatus(GLuint shaderProgram);

    void scaleVolumeAndNormalizeSize();

    void updateFieldOfView();
    void updateNoise();

    void updateCameraPosition();

    // global buffer handles
    GLuint m_vao_cube_vertices;
    GLuint m_vbo_cube_vertices;
    GLuint m_ibo_cube_elements;
    GLuint m_ibo_cube_lines_elements;
    // global shader hanldes
    GLuint m_vertexShaderRayCasting;
    GLuint m_fragmentShaderRayCasting;
    GLuint m_shaderProgramRayCasting;
    // bounding box shader handles
    GLuint m_vertexShaderBoundingBox;
    GLuint m_fragmentShaderBoundingBox;
    GLuint m_shaderProgramBoundingBox;

    // Matrices
    const QMatrix4x4* const m_projectionMatrix;
    const QMatrix4x4* const m_viewMatrix;
    QMatrix4x4 m_rotationMatrix;
    QMatrix4x4 m_translationMatrix;
    QMatrix4x4 m_scaleMatrix;

    // stores the volume data
    VolumeData3DTexture m_texture;

    // stores random jitter noise
    NoiseTexture2D m_noiseTexture;

    RaycastShaderSettings m_settings;

    bool m_renderBoundingBox;
};
} // namespace VDS