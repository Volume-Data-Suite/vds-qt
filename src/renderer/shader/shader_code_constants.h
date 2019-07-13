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

		"uniform mat4 viewModelMatrix; \n"
		"uniform mat4 projectionViewModelMatrix; \n"

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


		"// Ray \n"
		"struct Ray { \n"
		"	vec3 origin; \n"
		"	vec3 direction; \n"
		"}; \n"

		"// Axis-aligned bounding box \n"
		"struct AABB { \n"
		"	vec3 top; \n"
		"	vec3 bottom; \n"
		"}; \n"


		"// Slab method for ray-box intersection \n"
		"void RayBoxIntersection(const Ray ray, const AABB box, out float nearIntersectionPoint, out float farIntersectionPoint) \n"
		"{ \n"
		"	const vec3 invertedDirection = 1.0 / ray.direction; \n"
		"	const vec3 t_top = invertedDirection * (box.top - ray.origin); \n"
		"	const vec3 t_bottom = invertedDirection * (box.bottom - ray.origin); \n"
		"	const vec3 t_min = min(t_top, t_bottom); \n"
		"	vec2 t = max(t_min.xx, t_min.yz); \n"
		"	// clamp to zero for the near intersection, since negative values correspond to positions behind the camera \n"
		"	nearIntersectionPoint = max(0.0, max(t.x, t.y)); \n"
		"	const vec3 t_max = max(t_top, t_bottom); \n"
		"	t = min(t_max.xx, t_max.yz); \n"
		"	farIntersectionPoint = min(t.x, t.y); \n"
		"} \n"

		"void main() \n"
		"{ \n"
		"	vec3 ray_direction; \n"
		"	ray_direction.xy = 2.0 * gl_FragCoord.xy / viewportSize - 1.0; \n"
		"	ray_direction.x *= aspectRatio; \n"
		"	ray_direction.z = -focalLength; \n"
		"	ray_direction = (vec4(ray_direction, 0) * viewModelMatrix).xyz; \n"

		"	float nearIntersectionPoint, farIntersectionPoint; \n"
		"	const Ray casting_ray = Ray(rayOrigin, ray_direction); \n"
		"	const AABB bounding_box = AABB(topAABB, bottomAABB); \n"
		"	RayBoxIntersection(casting_ray, bounding_box, nearIntersectionPoint, farIntersectionPoint); \n"

		"	vec3 ray_start = (rayOrigin + ray_direction * nearIntersectionPoint - bottomAABB) / (topAABB - bottomAABB); \n"
		"	const vec3 ray_stop = (rayOrigin + ray_direction * farIntersectionPoint - bottomAABB) / (topAABB - bottomAABB); \n"

		"	vec3 ray = ray_stop - ray_start; \n"
		"	vec3 step_vector = normalize(ray) * sampleStepLength; \n"

		"	// Random jitter \n"
		"	ray_start += step_vector * texture(noiseTex, gl_FragCoord.xy / viewportSize).r; \n"
		
		"	vec3 position = ray_start; \n"

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

		//"		position += step_vector; \n"
		"	} \n"
			   
		"	fragColor.xyz = vec3(position); \n"
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
