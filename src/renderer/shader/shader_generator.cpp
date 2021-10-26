
#include <stdexcept>
#include "shader_code_constants.h"
#include "shader_generator.h"

namespace VDS {

const std::string ShaderGenerator::getVertexShaderCodeRaycasting() {
    std::string vertexShader = GLSL::vertrexBaseRaycasting;

    insertGLSLVerion(vertexShader);

    return vertexShader;
}
const std::string ShaderGenerator::getFragmentShaderCodeRaycasting(
    const RaycastShaderSettings& settings) {
    std::string fragmentShader = GLSL::fragmentBaseRaycasting;

    insertGLSLVerion(fragmentShader);
    insertRaycastMethod(fragmentShader, settings.method);
    insertApplyWindowMethod(fragmentShader, settings.windowSettings);
    insertPhongShading(fragmentShader, false);

    return fragmentShader;
}
const std::string ShaderGenerator::getVertexShaderCodeSlice2D() {
    std::string vertexShader = GLSL::vertrexBaseSlice2D;

    insertGLSLVerion(vertexShader);

    return vertexShader;
}
const std::string ShaderGenerator::getFragmentShaderCodeSlice2D(
    const Slice2DShaderSettings& settings) {
    std::string fragmentShader = GLSL::fragmentBaseSlice2D;

    insertGLSLVerion(fragmentShader);
    insertColorSlice2D(fragmentShader, settings.axis);

    return fragmentShader;
}

void ShaderGenerator::insertGLSLVerion(std::string& shader) {
    shader.replace(shader.find(GLSL::glslVersion.first), GLSL::glslVersion.first.length(),
                   GLSL::glslVersion.second);
}
void ShaderGenerator::insertRaycastMethod(std::string& shader, const RayCastMethods method) {
    switch (method) {
    case VDS::RayCastMethods::MIP: {
        shader.replace(shader.find(GLSL::raycastinMethodMID.first),
                       GLSL::raycastinMethodMID.first.length(), GLSL::raycastinMethodMID.second);
        break;
    }
    case VDS::RayCastMethods::LMIP: {
        shader.replace(shader.find(GLSL::raycastinMethodLMID.first),
                       GLSL::raycastinMethodLMID.first.length(), GLSL::raycastinMethodLMID.second);
        break;
    }
    case VDS::RayCastMethods::FirstHit: {
        shader.replace(shader.find(GLSL::raycastinMethodFirstHit.first),
                       GLSL::raycastinMethodFirstHit.first.length(),
                       GLSL::raycastinMethodFirstHit.second);
        break;
    }
    case VDS::RayCastMethods::FirstHitDepth: {
        shader.replace(shader.find(GLSL::raycastinMethodFirstHitDepth.first),
                       GLSL::raycastinMethodFirstHitDepth.first.length(),
                       GLSL::raycastinMethodFirstHitDepth.second);
        break;
    }
    case VDS::RayCastMethods::Accumulate: {
        shader.replace(shader.find(GLSL::raycastinMethodAccumulate.first),
                       GLSL::raycastinMethodAccumulate.first.length(),
                       GLSL::raycastinMethodAccumulate.second);
        break;
    }
    case VDS::RayCastMethods::Average: {
        shader.replace(shader.find(GLSL::raycastinMethodAverage.first),
                       GLSL::raycastinMethodAverage.first.length(),
                       GLSL::raycastinMethodAverage.second);
        break;
    }
    default: {
        std::runtime_error("unimplemented");
        break;
    }
    }
}
void ShaderGenerator::insertApplyWindowMethod(std::string& shader,
                                              const ValueWindowSettings& windowSettings) {
    if (windowSettings.enabled) {
        shader.replace(shader.find(GLSL::accessVoxelWithWindow.first),
                       GLSL::accessVoxelWithWindow.first.length(),
                       GLSL::accessVoxelWithWindow.second);

        switch (windowSettings.method) {
        case VDS::WindowingMethod::LinearExact: {
            shader.replace(shader.find(GLSL::applyWindowFunctionLinearExact.first),
                           GLSL::applyWindowFunctionLinearExact.first.length(),
                           GLSL::applyWindowFunctionLinearExact.second);
            break;
        }
        case VDS::WindowingMethod::Sigmoid: {
            shader.replace(shader.find(GLSL::applyWindowFunctionSigmoid.first),
                           GLSL::applyWindowFunctionSigmoid.first.length(),
                           GLSL::applyWindowFunctionSigmoid.second);
            break;
        }
        case VDS::WindowingMethod::Linear:
        default: {
            shader.replace(shader.find(GLSL::applyWindowFunctionLinear.first),
                           GLSL::applyWindowFunctionLinear.first.length(),
                           GLSL::applyWindowFunctionLinear.second);
            break;
        }
        }

    } else {
        shader.replace(shader.find(GLSL::applyWindowFunctionLinear.first),
                       GLSL::applyWindowFunctionLinear.first.length(), "");
        shader.replace(shader.find(GLSL::accessVoxelWithoutWindow.first),
                       GLSL::accessVoxelWithoutWindow.first.length(),
                       GLSL::accessVoxelWithoutWindow.second);
    }
}
void ShaderGenerator::insertPhongShading(std::string& shader, const bool precomputedGradients) {
    if (precomputedGradients) {
        shader.replace(shader.find(GLSL::getGradientOnTheFly.first),
                       GLSL::getGradientOnTheFly.first.length(), GLSL::getGradientOnTheFly.second);
    } else {
        shader.replace(shader.find(GLSL::getGradientOnTheFly.first),
                       GLSL::getGradientOnTheFly.first.length(), GLSL::getGradientOnTheFly.second);
    }
    shader.replace(shader.find(GLSL::getPhongShading.first), GLSL::getPhongShading.first.length(),
                   GLSL::getPhongShading.second);
}
void ShaderGenerator::insertColorSlice2D(std::string& shader, const VDTK::VolumeAxis axis) {
    std::string color;
    switch (axis) {
    case VDTK::VolumeAxis::YZAxis:
        color = "vec3(0.2f, 0.0f, 0.0f)";
        break;
    case VDTK::VolumeAxis::XZAxis:
        color = "vec3(0.0f, 0.2f, 0.0f)";
        break;
    case VDTK::VolumeAxis::XYAxis:
    default:
        color = "vec3(0.0f, 0.0f, 0.2f)";
        break;
    }

    shader.replace(shader.find("{{ color }}"), std::string("{{ color }}").length(), color);
}
} // namespace VDS