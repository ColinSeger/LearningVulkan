#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;//w is light intensity
} ubo;

layout(push_constant) uniform Push {
	//mat4 transform; //Projection * view * model
	mat4 modelMatrix;
	mat4 normalMatrix; //This is for if I need to have a non-uniform scale
} push;

//const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0,-3.0,-1.0));
//const float AMBIENT_LIGHT = 0.02;

void main()	{

	vec4 worldPosition = push.modelMatrix * vec4(position, 1.0);

	gl_Position = ubo.projectionViewMatrix * worldPosition;



	//gl_Position = push.transform * vec4(position, 1.0);

	//Does not supprot non-uniform scale
	//Is not computationally expensive
	//vec3 normalWorldSpace = normalize(mat3(push.modelMatrix) * normal);
	
	//supports non-uniform scale but computationally more expensive
	//mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
	//vec3 normalWorldSpace = normalize(normalMatrix * normal);

	//supports non-uniform scale
	//But is computationally more expensive if you have many objects

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	vec3 directionToLight = ubo.lightPosition - worldPosition.xyz;
	//vec3 directionToLight = normalize(ubo.lightPosition - worldPosition.xyz); //Try this later Remember to remove normalize from difuseLight
	//float atenuation = 1.0 / length(ubo.lightPosition - worldPosition.xyz);
	float atenuation = 1.0 / dot(directionToLight, directionToLight);

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * atenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 difuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0);

	fragColor = (difuseLight + ambientLight) * color;
}