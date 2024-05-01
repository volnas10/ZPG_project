#version 450 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 2) in vec2 VertexUV;

// ws - worldspace
// cs - cameraspace
// ms - modelspace
// ts - tangentspace

layout(std140, binding = 0) uniform ModelMatrices{
	mat4 model_matrix[1024];
};

uniform mat4 ProjectionViewMatrix;

out vec2 uv;

void main(){
	gl_Position = ProjectionViewMatrix * model_matrix[gl_InstanceID] * vec4(VertexPosition, 1);
	uv = VertexUV;
}



