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

		"uniform float focal_length; \n"
		"uniform float aspect_ratio; \n"
		"uniform vec2 viewport_size; \n"
		"uniform vec3 ray_origin; \n"
		"uniform vec3 top; \n"
		"uniform vec3 bottom; \n"

		"uniform sampler3D dataTex; \n"
		"uniform sampler2D noiseTex; \n"

		"uniform float sample_step_length; \n"

		"out vec4 fragColor; \n"


		"// Ray \n"
		"struct Ray { \n"
		"	const vec3 origin; \n"
		"	const vec3 direction; \n"
		"}; \n"

		"// Axis-aligned bounding box \n"
		"struct AABB { \n"
		"	const vec3 top; \n"
		"	const vec3 bottom; \n"
		"}; \n"


		"// Slab method for ray-box intersection \n"
		"void ray_box_intersection(Ray ray, AABB box, out float t_0, out float t_1) \n"
		"{ \n"
		"	const vec3 direction_inv = 1.0 / ray.direction; \n"
		"	const vec3 t_top = direction_inv * (box.top - ray.origin); \n"
		"	const vec3 t_bottom = direction_inv * (box.bottom - ray.origin); \n"
		"	const vec3 t_min = min(t_top, t_bottom); \n"
		"	vec2 t = max(t_min.xx, t_min.yz); \n"
		"	// clamp to zero for the near intersection, since negative values correspond to positions behind the camera \n"
		"	t_0 = max(0.0, max(t.x, t.y)); \n"
		"	const vec3 t_max = max(t_top, t_bottom); \n"
		"	t = min(t_max.xx, t_max.yz); \n"
		"	t_1 = min(t.x, t.y); \n"
		"} \n"

		"void main() \n"
		"{ \n"
		"vec3 ray_direction; \n"
		"ray_direction.xy = 2.0 * gl_FragCoord.xy / viewport_size - 1.0; \n"
		"ray_direction.x *= aspect_ratio; \n"
		"ray_direction.z = -focal_length; \n"
		"ray_direction = (vec4(ray_direction, 0) * viewModelMatrix).xyz; \n"

		"float t_0, t_1; \n"
		"const Ray casting_ray = Ray(ray_origin, ray_direction); \n"
		"const AABB bounding_box = AABB(top, bottom); \n"
		"ray_box_intersection(casting_ray, bounding_box, t_0, t_1); \n"

		"vec3 ray_start = (ray_origin + ray_direction * t_0 - bottom) / (top - bottom); \n"
		"const vec3 ray_stop = (ray_origin + ray_direction * t_1 - bottom) / (top - bottom); \n"

		"vec3 ray = ray_stop - ray_start; \n"
		"vec3 step_vector = normalize(ray) * sample_step_length; \n"

		"// Random jitter \n"
		"ray_start += step_vector * texture(noiseTex, gl_FragCoord.xy / viewport_size).r; \n"
		
		"vec3 position = ray_start; \n"

		"{{ raycastingMethod }} \n"
			   
		"fragColor.xyz = vec3(maximum_intensity); \n"
		"fragColor.w = 1.0f; \n"

		"} \n";


	static const std::pair<std::string, std::string> raycastinMethodMID = std::make_pair("{{ raycastingMethod }}", 
		"float maximum_intensity = 0.0; \n"
		
		"const int steps = int(length(ray) / sample_step_length); \n"
		"// Ray march until reaching the end of the volume \n"
		"for (int i = 0; i <= steps; i++) { \n"
		"	maximum_intensity = max(maximum_intensity, texture(dataTex, position).r); \n"

		"	position += step_vector; \n"
		"} \n"		
		);



}
