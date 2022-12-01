#version 460

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UV;

uniform mat4 PVmatrix;

layout (std140, binding=0) uniform ModelMatrices {
	mat4 model_matrix[1024];
};

out vec2 uv;

void main () {
	gl_Position = PVmatrix * model_matrix[gl_InstanceID] * vec4(VertexPosition, 1);
	uv = UV;
}