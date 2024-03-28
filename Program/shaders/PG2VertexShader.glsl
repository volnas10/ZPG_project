#version 450 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexUV;

// ws - worldspace
// cs - cameraspace
// ms - modelspace
// ts - tangentspace

out vec2 uv;
out vec3 vertexPosition_ws;
out vec3 normal_cs;
out vec4 eyeDirection_cs;
out vec3 normal_ws;
flat out vec3 eyePosition_ws;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionViewMatrix;

layout(std140, binding = 0) uniform ModelMatrices{
	mat4 model_matrix[1024];
};

void main(){
	gl_Position = ProjectionViewMatrix * model_matrix[gl_InstanceID] * vec4(VertexPosition, 1);
	
	vertexPosition_ws = (model_matrix[gl_InstanceID] * vec4(VertexPosition, 1)).xyz;
	
	vec4 vertexPosition_cs = (ViewMatrix * vec4(vertexPosition_ws, 1));

	mat4 modelViewMatrix = ViewMatrix * model_matrix[gl_InstanceID];

	mat4 normalMatrix = transpose(inverse(modelViewMatrix));

	normal_cs = normalize(vec3(normalMatrix * vec4(VertexNormal, 1)));

	eyeDirection_cs = normalize(vec3(0, 0, 0) - vertexPosition_cs);

	normal_ws = normalize(mat3(inverse(ViewMatrix)) * normal_cs);
	
	eyePosition_ws = vec3(inverse(ViewMatrix)[3]);

	uv = VertexUV;
}



