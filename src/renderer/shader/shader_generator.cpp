
#include <stdexcept>
#include "shader_code_constants.h"
#include "shader_generator.h"

namespace VDS {

const std::string ShaderGenerator::getFragmentShaderCode(const RaycastShaderSettings& settings) {
    std::string fragmentShader = GLSL::fragmentBase;

    insertGLSLVerion(fragmentShader);
    insertRaycastMethod(fragmentShader, settings.method);
    insertApplyWindowMethod(fragmentShader, settings.windowSettings);
    insertPhongShading(fragmentShader, false);
    insertLightSources(fragmentShader, settings.lightSources);

    return fragmentShader;
}
const std::string ShaderGenerator::getVertexShaderCode() {
    std::string vertexShader = GLSL::vertrexBase;

    insertGLSLVerion(vertexShader);

    return vertexShader;
}

void ShaderGenerator::insertGLSLVerion(std::string& shader) {
    shader.replace(shader.find(GLSL::glslVersion.first), GLSL::glslVersion.first.length(),
                   GLSL::glslVersion.second);
}
void ShaderGenerator::insertRaycastMethod(std::string& shader, RayCastMethods method) {
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
    case VDS::RayCastMethods::Accumulate:
    case VDS::RayCastMethods::Average:
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
void ShaderGenerator::insertLightSources(std::string& shader,
                                         const std::vector<std::array<float, 3>>& lightSources) {
    shader.replace(shader.find("{{ lightSourcesCount }}"),
                   std::string("{{ lightSourcesCount }}").length(),
                   std::to_string(lightSources.size()));
}
} // namespace VDS