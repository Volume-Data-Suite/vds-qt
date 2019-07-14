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
		"uniform float threshold; \n"

		"uniform float valueWindowWidth; \n"
		"uniform float valueWindowCenter; \n"
		"uniform float valueWindowOffset; \n"

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
		"} \n"

		"{{ applyWindowFunction }}"

		"float getVolumeValue(vec3 position) { \n"
		"	return {{ accessVoxel }}; \n"
		"} \n"
		
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
		"	float maximum_intensity = 0.0f; \n"
		
		"	const int steps = int(length(ray) / sampleStepLength); \n"
		"	// Ray march until reaching the end of the volume \n"
		"	for (int i = 0; i <= steps; i++) { \n"
		"		maximum_intensity = max(maximum_intensity, getVolumeValue(position)); \n"

		"		position += step_vector; \n"
		"	} \n"
		"	fragColor = vec4(vec3(maximum_intensity), 1.0f); \n"
		);

	static const std::pair<std::string, std::string> raycastinMethodLMID = std::make_pair("{{ raycastingMethod }}",
		"	float maximum_intensity = 0.0f; \n"

		"	const int steps = int(length(ray) / sampleStepLength); \n"
		"	// Ray march until reaching the end of the volume \n"
		"	for (int i = 0; i <= steps; i++) { \n"
		"		const float intensity = getVolumeValue(position); \n"

		"		if(intensity >= threshold) { \n"
		"			if(intensity >= maximum_intensity) { \n"
		"				maximum_intensity = intensity; \n"
		"			} \n"
		"			else { \n"
		"				break; \n"
		"			} \n"
		"		} \n"


		"		position += step_vector; \n"
		"	} \n"

		"	fragColor = vec4(maximum_intensity); \n"
		"	fragColor.w = 1.0f; \n"
	);


	static const std::pair<std::string, std::string> raycastinMethodFirstHit = std::make_pair("{{ raycastingMethod }}",
		"	float firstHit = 0.0f; \n"

		"	const int steps = int(length(ray) / sampleStepLength); \n"
		"	// Ray march until reaching the end of the volume \n"
		"	for (int i = 0; i <= steps; i++) { \n"
		"		float intensity = getVolumeValue(position); \n"

		"		if(intensity >= threshold) { \n"
		"			firstHit = intensity; \n"
		"			break; \n"
		"		} \n"

		"		position += step_vector; \n"
		"	} \n"

		"	fragColor.xyz = vec3(firstHit); \n"
		"	fragColor.w = 1.0f; \n"
	);


	static const std::pair<std::string, std::string> applyWindowFunction = std::make_pair("{{ applyWindowFunction }}",
		"float applyWindow(float inputValue) { \n"
		"	const float UINT16MAX = 65535.0f; \n"
		"	const float UINT16STEPTOFLOAT = 1.0f / UINT16MAX; \n"

		"	const float lowerBorder = valueWindowCenter - (UINT16STEPTOFLOAT * 0.5f) - ((valueWindowWidth - UINT16STEPTOFLOAT) / (2.0f * UINT16STEPTOFLOAT)); \n"
		"	const float upperBorder = valueWindowCenter - (UINT16STEPTOFLOAT * 0.5f) + ((valueWindowWidth - UINT16STEPTOFLOAT) / (2.0f * UINT16STEPTOFLOAT)); \n"
		"	const float mappedValue = ((inputValue + valueWindowOffset) - (valueWindowCenter - (UINT16STEPTOFLOAT * 0.5f))) / (valueWindowWidth - UINT16STEPTOFLOAT) + (UINT16STEPTOFLOAT * 0.5f); \n"

		"	const float lowerBorderFactor = float(inputValue + valueWindowOffset <= lowerBorder); \n"
		"	const float upperBorderFactor = float(inputValue + valueWindowOffset > upperBorder); \n"
				
		"	return (mappedValue * (1.0f - lowerBorderFactor)) * (1.0f - upperBorderFactor) + upperBorderFactor; \n"
		"} \n"
	);

	static const std::pair<std::string, std::string> accessVoxelWithoutWindow = std::make_pair("{{ accessVoxel }}",
		"texture(dataTex, position).r"
	);

	static const std::pair<std::string, std::string> accessVoxelWithWindow = std::make_pair("{{ accessVoxel }}",
		"applyWindow(texture(dataTex, position).r)"
	);




//#ifdef QT_RELEASE
//	+ uint16_t * ptr = (uint16_t*)bytes.data();
//	+float c = 40;
//	+float w = 450;
//	+float yMin = 0;
//	+float offset = -1000;
//	+float yMax = UINT16_MAX;
//	+for (size_t i = 0; i < m_p.dimensions.x() * m_p.dimensions.y() * m_p.dimensions.z(); i++)
//		+ {
//		+if (ptr[i] + offset <= c - 0.5 - ((w - 1) / 2))
//			+ {
//			+ptr[i] = yMin;
//			+}
//		+else if (ptr[i] + offset > c - 0.5 + ((w - 1) / 2))
//			+ {
//			+ptr[i] = yMax;
//			+}
//		+else
//			+ {
//			+ptr[i] = (((ptr[i] + offset) - (c - 0.5)) / (w - 1) + 0.5) * (yMax - yMin) + yMin;
//			+}
//		+}
//	+#endif // QT_DEBUG


}
