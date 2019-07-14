#pragma once
#include <string>
#include "shader_settings.h"

namespace VDS
{
	class ShaderGenerator
	{
	public:
		ShaderGenerator() = default;
		~ShaderGenerator() = default;

		static const std::string getVertexShaderCode();
		static const std::string getFragmentShaderCode(const RaycastShaderSettings& settings);

	private:
		static void insertGLSLVerion(std::string& shader);
		static void insertRaycastMethod(std::string& shader, RayCastMethods method);
		static void insertApplyWindowMethod(std::string& shader, const ValueWindowSettings& windowSettings);

	};

	
}
