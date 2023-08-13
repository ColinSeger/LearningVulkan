#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWorldSpace;
layout(location = 2) in vec3 fragNormalWorldSpace;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;//w is light intensity
} ubo;

layout(push_constant) uniform Push {
	//mat4 transform; //Projection * view * model
	mat4 modelMatrix;
	mat4 normalMatrix; //This is for if I need to have a non-uniform scale
} push;

void main() {
	vec3 directionToLight = ubo.lightPosition - fragPositionWorldSpace;

	float atenuation = 1.0 / dot(directionToLight, directionToLight);

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * atenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 difuseLight = lightColor * max(dot(normalize(fragNormalWorldSpace), normalize(directionToLight)), 0);

	outColor = vec4((difuseLight + ambientLight) * fragColor, 1.0);
}