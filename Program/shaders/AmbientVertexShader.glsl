#version 450 core

layout(location = 0) in vec3 VertexPosition;

// ws - worldspace
// cs - cameraspace
// ms - modelspace
// ts - tangentspace

layout(std140, binding = 0) uniform ModelMatrices{
	mat4 model_matrix[1024];
};

void main(){
	gl_Position = model_matrix[gl_InstanceID] * vec4(VertexPosition, 1);
}



