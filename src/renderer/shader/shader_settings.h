#pragma once
#include <array>

namespace VDS {
enum class RayCastMethods {
    MIP,
    LMIP,
    FirstHit,
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

	bool phongShading = true;
};
} // namespace VDS