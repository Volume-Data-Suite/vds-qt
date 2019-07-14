#pragma once
#include <array>

namespace VDS
{
	enum class RayCastMethods
	{
		MIP,
		LMIP,
		FirstHit,
		Accumulate,
		Average,
	};

	struct ValueWindowSettings
	{
		bool enabled = false;
		float valueWindowWidth = (1.0f / static_cast<float>(UINT16_MAX)) * 450.0f;
		float valueWindowCenter = (1.0f / static_cast<float>(UINT16_MAX)) * 40.0f;
		float valueWindowOffset = (1.0f / static_cast<float>(UINT16_MAX)) * -1000.0f;
	};

	struct RaycastShaderSettings
	{
		RayCastMethods method = RayCastMethods::MIP;
		ValueWindowSettings windowSettings;

		float aspectRationOpenGLWindow = 1.0f;
		std::array<float, 2> viewportSize = { 100.0f, 100.0f};

		float sampleStepLength = 0.01f;;
		float threshold = 0.05f;
	};
}