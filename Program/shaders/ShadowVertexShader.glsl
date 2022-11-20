#version 460

layout(location = 0) in vec3 VertexPosition;

uniform mat4 PVmatrix;

layout (std140, binding=0) uniform ModelMatrices {
	mat4 model_matrix[1024];
};

void main () {
	gl_Position = PVmatrix * model_matrix[gl_InstanceID] * vec4(VertexPosition, 1);
}