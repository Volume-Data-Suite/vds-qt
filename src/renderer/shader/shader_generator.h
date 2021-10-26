#pragma once
#include <string>
#include "shader_settings.h"
#include <VDTK/common/CommonDataTypes.h>

namespace VDS {
class ShaderGenerator {
public:
    ShaderGenerator() = default;
    ~ShaderGenerator() = default;

    // Volume View
    static const std::string getVertexShaderCodeRaycasting();
    static const std::string getFragmentShaderCodeRaycasting(const RaycastShaderSettings& settings);

    // Slice View
    static const std::string getVertexShaderCodeSlice2D();
    static const std::string getFragmentShaderCodeSlice2D(const Slice2DShaderSettings& settings);

private:
    static void insertGLSLVerion(std::string& shader);
    static void insertRaycastMethod(std::string& shader, const RayCastMethods method);
    static void insertApplyWindowMethod(std::string& shader,
                                        const ValueWindowSettings& windowSettings);
    static void insertPhongShading(std::string& shader, const bool active);

    static void insertColorSlice2D(std::string& shader, const VDTK::VolumeAxis axis);
};

} // namespace VDS
