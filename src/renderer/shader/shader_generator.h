#pragma once
#include <string>

namespace VDS
{
	class ShaderGenerator
	{
	public:
		ShaderGenerator() = default;
		~ShaderGenerator() = default;

		static const std::string getVertexShaderCode();
		static const std::string getFragmentShaderCode();

	private:
		static void insertGLSLVerion(std::string& shader);

	};

	
}
