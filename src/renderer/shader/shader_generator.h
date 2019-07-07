#pragma once
#include <string>

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

	struct RaycastShaderSettings
	{
		RayCastMethods method = RayCastMethods::MIP;
	};

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

	};

	
}
