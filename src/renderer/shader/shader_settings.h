#pragma once
#include <array>
#include <vector>

#include <VDTK/common/CommonDataTypes.h>

namespace VDS {
enum class RayCastMethods {
    MIP,
    LMIP,
    FirstHit,
    FirstHitDepth,
    Accumulate,
    Average,
};

// VOI LUT functions
enum class WindowingMethod { Linear, LinearExact, Sigmoid };

struct ValueWindowSettings {
    bool enabled = false;
    WindowingMethod method = WindowingMethod::Linear;
    float valueWindowWidth = 1.0f;
    float valueWindowCenter = 0.5f;
    float valueWindowOffset = 0.0f;
};

struct RaycastShaderSettings {
    RayCastMethods method = RayCastMethods::MIP;
    ValueWindowSettings windowSettings;

    float aspectRationOpenGLWindow = 1.0f;
    std::array<float, 2> viewportSize = {100.0f, 100.0f};

    float sampleStepLength = 0.01f;
    float threshold = 0.05f;
};

struct Slice2DShaderSettings {
    ValueWindowSettings windowSettings;
    VDTK::VolumeAxis axis = VDTK::VolumeAxis::XYAxis;
    VDTK::VolumeSize size = VDTK::VolumeSize{0, 0, 0};
    VDTK::VolumeSpacing spacing = VDTK::VolumeSpacing{1.0, 1.0, 1.0};
    std::array<float, 2> viewportSize = {100.0f, 100.0f};
    std::uint32_t position = 0;
};

} // namespace VDS