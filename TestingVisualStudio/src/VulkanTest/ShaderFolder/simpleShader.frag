#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	//mat4 transform; //Projection * view * model
	mat4 modelMatrix;
	mat4 normalMatrix; //This is for if I need to have a non-uniform scale
} push;

void main() {
	outColor = vec4(fragColor, 1.0);
}