
#include "shader_generator.h"
#include "shader_code_constants.h"


namespace VDS
{

	const std::string ShaderGenerator::getFragmentShaderCode(const RaycastShaderSettings& settings)
	{
		std::string fragmentShader = GLSL::fragmentBase;

		insertGLSLVerion(fragmentShader);
		insertRaycastMethod(fragmentShader, settings.method);

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
	void ShaderGenerator::insertRaycastMethod(std::string & shader, RayCastMethods method)
	{
		switch (method)
		{
		case VDS::RayCastMethods::MIP:
		{
			shader.replace(shader.find(GLSL::raycastinMethodMID.first), GLSL::raycastinMethodMID.first.length(), GLSL::raycastinMethodMID.second);
			break;
		}
		case VDS::RayCastMethods::LMIP:
		case VDS::RayCastMethods::FirstHit:
		case VDS::RayCastMethods::Accumulate:
		case VDS::RayCastMethods::Average:
		default:
		{
			std::runtime_error("unimplemented");
			break;
		}
		}
	}
}