
#include "shader_generator.h"
#include "shader_code_constants.h"


namespace VDS
{

	const std::string ShaderGenerator::getFragmentShaderCode()
	{
		std::string fragmentShader = GLSL::fragmentBase;

		insertGLSLVerion(fragmentShader);

		return fragmentShader;
	}
	const std::string ShaderGenerator::getVertexShaderCode()
	{
		std::string vertexShader = GLSL::vertrexBase;

		insertGLSLVerion(vertexShader);

		return vertexShader;
	}

	void ShaderGenerator::insertGLSLVerion(std::string & shader)
	{
		shader.replace(shader.find(GLSL::glslVersion.first), GLSL::glslVersion.first.length(), GLSL::glslVersion.second);
	}
}