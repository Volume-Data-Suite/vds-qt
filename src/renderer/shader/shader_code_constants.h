#pragma once
#include <string>

namespace VDS::GLSL
{
	static const std::pair<std::string, std::string> glslVersion = std::make_pair("{{ glslVersion }}", "#version 430 core" );
		
	static const std::string vertrexBase = 
		glslVersion.first + "\n"
		
		"in vec3 inPos; \n"
		"uniform mat4 projectionViewModelMatrix; \n"

		"void main() \n"
		"{ \n"
		"	gl_Position = projectionViewModelMatrix * vec4(inPos.x, inPos.y, inPos.z, 1.0f); \n"
		"} \n";


	static const std::string fragmentBase =
		glslVersion.first + "\n"

		"uniform mat4 viewModelMatrixWithoutModleScale; \n"

		"uniform float focalLength; \n"
		"uniform float aspectRatio; \n"
		"uniform vec2 viewportSize; \n"
		"uniform vec3 rayOrigin; \n"
		"uniform vec3 topAABB; \n"
		"uniform vec3 bottomAABB; \n"

		"uniform sampler3D dataTex; \n"
		"uniform sampler2D noiseTex; \n"

		"uniform float sampleStepLength; \n"

		"out vec4 fragColor; \n"
			   
		"vec2 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) { \n"
		"	const vec3 tMin = (boxMin - rayOrigin) / rayDir; \n"
		"	const vec3 tMax = (boxMax - rayOrigin) / rayDir; \n"
		"	const vec3 t1 = min(tMin, tMax); \n"
		"	const vec3 t2 = max(tMin, tMax); \n"
		"	const float tFar = min(min(t2.x, t2.y), t2.z); \n"
		"	float tNear = max(max(t1.x, t1.y), t1.z); \n"
		"	// clamp tNear to 0.0f, incase the camera is inside the volume \n"
		"	tNear = max(0.0f, tNear); \n"
		"	return vec2(tNear, tFar); \n"
		"}; \n"
		

		"void main() \n"
		"{ \n"
		"	vec3 ray_direction; \n"
		"	ray_direction.xy = (2.0 * gl_FragCoord.xy / viewportSize - 1.0); \n"
		"	ray_direction.x *= aspectRatio; \n"
		"	ray_direction.z = -focalLength; \n"
		"	ray_direction = (vec4(ray_direction, 0) * viewModelMatrixWithoutModleScale).xyz; \n"

		"	const vec2 intersection = intersectAABB(rayOrigin, ray_direction, bottomAABB, topAABB); \n"

		"	vec3 ray_start = (rayOrigin + ray_direction * intersection.x + topAABB) / (topAABB - bottomAABB); \n"
		"	vec3 ray_stop = (rayOrigin + ray_direction * intersection.y + topAABB) / (topAABB - bottomAABB); \n"

		"	vec3 ray = ray_stop - ray_start; \n"
		"	vec3 step_vector = normalize(ray) * sampleStepLength; \n"

		"	// Random jitter \n"		
		"	vec3 position = ray_start + step_vector * texture(noiseTex, gl_FragCoord.xy / viewportSize).r; \n"

		"{{ raycastingMethod }} \n"

		"} \n";


	static const std::pair<std::string, std::string> raycastinMethodMID = std::make_pair("{{ raycastingMethod }}", 
		"	float maximum_intensity = texture(dataTex, position).r; \n"
		
		"	const int steps = int(length(ray) / sampleStepLength); \n"
		"	// Ray march until reaching the end of the volume \n"
		"	for (int i = 0; i <= steps; i++) { \n"

		"		vec3 tmpPos = position; \n"
		"		//tmpPos.x *= 2.0f; \n"

		"		maximum_intensity = max(maximum_intensity, texture(dataTex, tmpPos).r); \n"

		"		position += step_vector; \n"
		"	} \n"
		"	fragColor.xyz = vec3(maximum_intensity); \n"
		"	fragColor.w = 1.0f; \n"
		);

	static const std::pair<std::string, std::string> raycastinMethodLMID = std::make_pair("{{ raycastingMethod }}",
		"	float maximum_intensity = 0.0f; \n"

		"	const int steps = int(length(ray) / sampleStepLength); \n"
		"	// Ray march until reaching the end of the volume \n"
		"	for (int i = 0; i <= steps; i++) { \n"
		"		const float intensity = texture(dataTex, position).r; \n"
		"		if(intensity >= maximum_intensity) { \n"
		"			maximum_intensity = intensity; \n"
		"		} \n"
		"		else { \n"
		"			break; \n"
		"		} \n"

		"		position += step_vector; \n"
		"	} \n"

		"	fragColor.xyz = vec3(maximum_intensity); \n"
		"	fragColor.w = 1.0f; \n"
	);



}
