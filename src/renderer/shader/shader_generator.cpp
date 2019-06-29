
#include "shader_generator.h"
#include "shader_code_constants.h"


namespace VDS
{

	const std::string ShaderGenerator::getFragmentShaderCode()
	{
		std::string fragmentShader = VDS::GLSL::fragmentBase;

		insertGLSLVerion(fragmentShader);

		return fragmentShader;
	}
	const std::string ShaderGenerator::getVertexShaderCode()
	{
		std::string vertexShader = VDS::GLSL::vertrexBase;

		insertGLSLVerion(vertexShader);

		return vertexShader;
	}

	void ShaderGenerator::insertGLSLVerion(std::string & shader)
	{
		shader.replace(shader.find(VDS::GLSL::glslVersion.first), VDS::GLSL::glslVersion.first.length(), VDS::GLSL::glslVersion.second);
	}
}