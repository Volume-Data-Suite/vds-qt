#pragma once
#include <string>

namespace VDS::GLSL
{
	static const std::pair<std::string, std::string> glslVersion = std::make_pair("{{ glslVersion }}", "#version 430 core" );
		
	static const std::string vertrexBase = 
		glslVersion.first + "\n"
		
		"layout(location = 0) in vec3 inPos; \n"
		"// Model Matrix \n"
		"layout(location = 1) uniform mat4 mMatrix; \n"
		"// View Matrix \n"
		"layout(location = 2) uniform mat4 vMatrix; \n"
		"// Projection Matrix \n"
		"layout(location = 3) uniform mat4 pMatrix; \n"

		"void main() \n"
		"{ \n"
		"	gl_Position = pMatrix * vMatrix * mMatrix * vec4(inPos.x, inPos.y, inPos.z, 1.0f); \n"
		"} \n";


	static const std::string fragmentBase =
		glslVersion.first + "\n"

		"uniform sampler3D dataTex; \n"
		"out vec4 FragColor; \n"

		"void main() \n"
		"{ \n"
		"	const vec3 position = vec3(0.5f); \n"
		"	const float dataValue = texture(dataTex, position).x; \n"
		"	FragColor = vec4(vec3(dataValue), 1.0f); //vec4(1.0f); \n"
		"} \n";



}
