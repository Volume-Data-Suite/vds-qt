#pragma once
#include <string>

namespace VDS::GLSL {
static const std::pair<std::string, std::string> glslVersion =
    std::make_pair("{{ glslVersion }}", "#version 430 core");

static const std::string vertrexBaseSlice2D =
    glslVersion.first + "\n"

                        "in vec3 inPos; \n"

                        "void main() \n"
                        "{ \n"
                        "	gl_Position = vec4(inPos, 1.0f); \n"
                        "} \n";

static const std::string fragmentBaseSlice2D = glslVersion.first +
                                               "\n"

                                               "uniform sampler3D dataTex; \n"
                                               "uniform float threshold; \n"

                                               "uniform vec2 viewport; \n"

                                               "uniform float position; \n"

                                               "uniform float valueWindowWidth; \n"
                                               "uniform float valueWindowCenter; \n"
                                               "uniform float valueWindowOffset; \n"

                                               "out vec4 FragColor; \n"

                                               "{{ applyWindowFunction }} \n"

                                               "float getVolumeValue(vec3 position) { \n"
                                               "	return {{ accessVoxel }}; \n"
                                               "} \n"

                                               "void main() \n"
                                               "{ \n"
                                               "	FragColor = vec4(vec3(getVolumeValue({{ position }})), 1.0f); \n "
                                               "	//FragColor = vec4({{ //position }}, 1.0f); \n "
                                               "} \n";

static const std::string vertrexBaseRaycasting =
    glslVersion.first +
    "\n"

    "in vec3 inPos; \n"
    "uniform mat4 projectionViewModelMatrix; \n"

    "void main() \n"
    "{ \n"
    "	gl_Position = projectionViewModelMatrix * vec4(inPos.x, inPos.y, inPos.z, 1.0f); \n"
    "} \n";

static const std::string fragmentBaseRaycasting =
    glslVersion.first +
    "\n"

    "uniform mat4 viewModelMatrixWithoutModleScale; \n"

    "uniform float focalLength; \n"
    "uniform float aspectRatio; \n"
    "uniform vec2 viewportSize; \n"
    "uniform vec3 rayOrigin; \n"
    "uniform vec3 topAABB; \n"
    "uniform vec3 bottomAABB; \n"

    "uniform vec3 cameraPosition; \n"

    "uniform sampler3D dataTex; \n"
    "uniform sampler2D noiseTex; \n"

    "uniform float sampleStepLength; \n"
    "uniform float threshold; \n"

    "uniform float valueWindowWidth; \n"
    "uniform float valueWindowCenter; \n"
    "uniform float valueWindowOffset; \n"

    "out vec4 fragColor; \n"
    "out float gl_FragDepth; \n"

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

    "{{ applyWindowFunction }} \n"

    "float getVolumeValue(vec3 position) { \n"
    "	return {{ accessVoxel }}; \n"
    "} \n"

    "{{ getGradient }} \n"
    "{{ getPhongShading }} \n"

    "void main() \n"
    "{ \n"
    "	vec3 ray_direction; \n"
    "	ray_direction.xy = (2.0 * gl_FragCoord.xy / viewportSize - 1.0); \n"
    "	ray_direction.x *= aspectRatio; \n"
    "	ray_direction.z = -focalLength; \n"
    "	ray_direction = (vec4(ray_direction, 0) * viewModelMatrixWithoutModleScale).xyz; \n"

    "	const vec2 intersection = intersectAABB(rayOrigin, ray_direction, bottomAABB, topAABB); \n"

    "	vec3 ray_start = (rayOrigin + ray_direction * intersection.x + topAABB) / (topAABB - "
    "bottomAABB); \n"
    "	vec3 ray_stop = (rayOrigin + ray_direction * intersection.y + topAABB) / (topAABB - "
    "bottomAABB); \n"

    "	vec3 ray = ray_stop - ray_start; \n"
    "	vec3 step_vector = normalize(ray) * sampleStepLength; \n"

    "	// Random jitter \n"
    "	vec3 jitter = step_vector * texture(noiseTex, gl_FragCoord.xy / viewportSize).r; \n"
    "	vec3 position = ray_start + jitter; \n"

    "	const int steps = int(length(ray - jitter) / sampleStepLength); \n"

    "{{ raycastingMethod }} \n"

    "	gl_FragDepth = length(position - ray_start) / gl_FragCoord.w; \n"

    "} \n";

static const std::pair<std::string, std::string> raycastinMethodMID = std::make_pair(
    "{{ raycastingMethod }}", "	float maximum_intensity = 0.0f; \n"
                              "	vec3 maximum_intensity_position = position; \n"

                              "	// Ray march until reaching the end of the volume \n"
                              "	for (int i = 0; i <= steps; i++) { \n"
                              "		const float current_value = getVolumeValue(position); \n"
                              "		maximum_intensity_position = (current_value > "
                              "maximum_intensity) ? position : maximum_intensity_position; \n"
                              "		maximum_intensity = max(maximum_intensity, current_value); \n"

                              "		position += step_vector; \n"
                              "	} \n"
                              "	position = maximum_intensity_position; \n"
                              "	fragColor = vec4(vec3(maximum_intensity), 1.0f); \n");

static const std::pair<std::string, std::string> raycastinMethodLMID = std::make_pair(
    "{{ raycastingMethod }}", "	float maximum_intensity = 0.0f; \n"

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

                              "	fragColor.xyz = vec3(maximum_intensity); \n"
                              "	fragColor.w = (maximum_intensity > 0.0f) ? 1.0f : 0.0f; \n");

static const std::pair<std::string, std::string> raycastinMethodFirstHit =
    std::make_pair("{{ raycastingMethod }}",
                   "	vec3 firstHit = vec3(0.0f); \n"

                   "	// Ray march until reaching the end of the volume \n"
                   "	float intensity = 0.0f; \n"
                   "	for (int i = 0; i <= steps; i++) { \n"
                   "		intensity = getVolumeValue(position); \n"

                   "		if(intensity >= threshold) { \n"
                   "			firstHit = vec3((intensity > 0.0f) ? 0.5f : 0.0f); \n"
                   "			break; \n"
                   "		} \n"

                   "		position += step_vector; \n"
                   "	} \n"

                   "	// Phong Shading \n"
                   "		firstHit += 0.5f * phongShading(ray, position, cameraPosition); \n"

                   "	fragColor.xyz = firstHit; \n"
                   "	fragColor.w = (intensity >= threshold) ? 1.0f : 0.0f; \n");

static const std::pair<std::string, std::string> raycastinMethodFirstHitDepth = std::make_pair(
    "{{ raycastingMethod }}",
    "	vec3 firstHit = vec3(0.0f); \n"

    "	// Ray march until reaching the end of the volume \n"
    "	float intensity = 0.0f; \n"
    "	for (int i = 0; i <= steps; i++) { \n"
    "		intensity = getVolumeValue(position); \n"

    "		if(intensity >= threshold) { \n"
    "			firstHit = vec3((intensity > 0.0f) ? 0.5f : 0.0f); \n"
    "			break; \n"
    "		} \n"

    "		position += step_vector; \n"
    "	} \n"

    "	fragColor.xyz = position + 0.2f * phongShading(ray, position, cameraPosition); \n"
    "	fragColor.w = 1.0f; \n");

static const std::pair<std::string, std::string> raycastinMethodAccumulate =
    std::make_pair("{{ raycastingMethod }}",
                   "	vec4 result = vec4(0.0f); \n"

                   "	// Ray march until reaching the end of the volume \n"
                   "	float intensity = 0.0f; \n"
                   "	for (int i = 0; i <= steps; i++) { \n"
                   "		intensity = getVolumeValue(position); \n"

                   "		if(intensity >= threshold) { \n"
                   "	    	vec3 color = 2.0f * phongShading(ray, position, cameraPosition); \n"
                   "	    	result += vec4((color / steps), 1.0f); \n"
                   "		} \n"

                   "		position += step_vector; \n"
                   "	} \n"

                   "	fragColor = result; \n");

static const std::pair<std::string, std::string> raycastinMethodAverage = std::make_pair(
    "{{ raycastingMethod }}", "	float sum = 0.0f; \n"

                              "	// Ray march until reaching the end of the volume \n"
                              "	for (int i = 0; i <= steps; i++) { \n"
                              "		sum += getVolumeValue(position); \n"
                              "		position += step_vector; \n"
                              "	} \n"
                              "	fragColor = vec4(vec3(sum / steps), 1.0f); \n");

static const std::pair<std::string, std::string> applyWindowFunctionLinear = std::make_pair(
    "{{ applyWindowFunction }}",
    "float applyWindow(float inputValue) { \n"
    "	const float UINT16MAX = 65535.0f; \n"
    "	const float UINT16STEPTOFLOAT = 1.0f / UINT16MAX; \n"

    "	const float windowCenterShifted = valueWindowCenter - (UINT16STEPTOFLOAT * 0.5f); \n"
    "	const float windowWidthShifted = valueWindowWidth - UINT16STEPTOFLOAT; \n"

    "	const float lowerBorder = windowCenterShifted - (windowWidthShifted * 0.5f); \n"
    "	const float upperBorder = windowCenterShifted + (windowWidthShifted * 0.5f); \n"

    "	const float mappedValue = (inputValue + valueWindowOffset - windowCenterShifted) / "
    "windowWidthShifted + 0.5f; \n"

    "	const float lowerBorderFactor = float(inputValue + valueWindowOffset <= lowerBorder); \n"
    "	const float upperBorderFactor = float(inputValue + valueWindowOffset > upperBorder); \n"

    "	const float result = (mappedValue * (1.0f - lowerBorderFactor)) * (1.0f - "
    "upperBorderFactor) + upperBorderFactor; \n"

    "	return clamp(result, 0.0f, 1.0f); \n"
    "} \n");

static const std::pair<std::string, std::string> applyWindowFunctionLinearExact = std::make_pair(
    "{{ applyWindowFunction }}",
    "float applyWindow(float inputValue) { \n"
    "	const float UINT16MAX = 65535.0f; \n"
    "	const float UINT16STEPTOFLOAT = 1.0f / UINT16MAX; \n"

    "	const float lowerBorder = valueWindowCenter - (valueWindowWidth * 0.5f); \n"
    "	const float upperBorder = valueWindowCenter + (valueWindowWidth * 0.5f); \n"

    "	const float mappedValue = (inputValue + valueWindowOffset - valueWindowCenter) / "
    "valueWindowWidth + 0.5f; \n"

    "	const float lowerBorderFactor = float(inputValue + valueWindowOffset <= lowerBorder); \n"
    "	const float upperBorderFactor = float(inputValue + valueWindowOffset > upperBorder); \n"

    "	const float result = (mappedValue * (1.0f - lowerBorderFactor)) * (1.0f - "
    "upperBorderFactor) + upperBorderFactor; \n"

    "	return clamp(result, 0.0f, 1.0f); \n"
    "} \n");

static const std::pair<std::string, std::string> applyWindowFunctionSigmoid =
    std::make_pair("{{ applyWindowFunction }}",
                   "float applyWindow(float inputValue) { \n"
                   "	const float UINT16MAX = 65535.0f; \n"

                   "	const float result = UINT16MAX / (1.0f + exp(-4.0f * ((inputValue + "
                   "valueWindowOffset - valueWindowCenter) / valueWindowWidth))); \n"

                   "	return clamp(result / UINT16MAX, 0.0f, 1.0f); \n"
                   "} \n");

static const std::pair<std::string, std::string> accessVoxelWithoutWindow =
    std::make_pair("{{ accessVoxel }}", "texture(dataTex, position).r");

static const std::pair<std::string, std::string> accessVoxelWithWindow =
    std::make_pair("{{ accessVoxel }}", "applyWindow(texture(dataTex, position).r)");

static const std::pair<std::string, std::string> getGradientOnTheFly = std::make_pair(
    "{{ getGradient }}", "vec3 getGradient(vec3 position) { \n"
                         "	const float d = sampleStepLength; \n"
                         "	const vec3 top = vec3(getVolumeValue(position + vec3(d, 0.0f, 0.0f)), "
                         "getVolumeValue(position + vec3(0.0f, d, 0.0f)), getVolumeValue(position "
                         "+ vec3(0.0f, 0.0f, d))); \n"
                         "	const vec3 bottom = vec3(getVolumeValue(position - vec3(d, 0.0f, "
                         "0.0f)), getVolumeValue(position - vec3(0.0f, d, 0.0f)), "
                         "getVolumeValue(position - vec3(0.0f, 0.0f, d))); \n"
                         "	return normalize(top - bottom); \n"
                         "} \n");

static const std::pair<std::string, std::string> getPhongShading = std::make_pair(
    "{{ getPhongShading }}", "vec3 phongShading(vec3 ray, vec3 position, vec3 lightPosition) { \n"
                             "	// Blinn-Phong shading \n"
                             "	const vec3 Ka = vec3(0.1, 0.1, 0.1); // ambient \n"
                             "	const vec3 Kd = vec3(0.6, 0.6, 0.6); // diffuse \n"
                             "	const vec3 Ks = vec3(0.2, 0.2, 0.2); // specular \n"
                             "	const float shininess = 100.0; \n"

                             "	// light properties \n"
                             "	const vec3 lightColor = vec3(1.0, 1.0, 1.0); \n"
                             "	const vec3 ambientLight = vec3(0.3, 0.3, 0.3); \n"

                             "	const vec3 L = normalize(lightPosition - position); \n"
                             "	const vec3 V = -normalize(ray); \n"
                             "	const vec3 N = getGradient(position); \n"
                             "	const vec3 H = normalize(L + V); \n"

                             "	// Compute ambient term \n"
                             "	const vec3 ambient = Ka * ambientLight; \n"
                             "	// Compute the diffuse term \n"
                             "	const float diffuseLight = max(dot(L, N), 0); \n"
                             "	const vec3 diffuse = Kd * lightColor * diffuseLight; \n"
                             "	// Compute the specular term \n"
                             "	const float specularLight = diffuseLight <= 0 ? 0.0f : "
                             "pow(max(dot(H, N), 0), shininess); \n"
                             "	const vec3 specular = Ks * lightColor * specularLight; \n"
                             "	return ambient + diffuse + specular; \n"
                             "} \n");

} // namespace VDS::GLSL
