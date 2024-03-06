#version 330 core

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
out vec3 eyeDirection_cs;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 MeshMatrix;

out vec3 normal_ws;

void main(){
	mat4 meshModelMatrix = ModelMatrix * MeshMatrix;

	gl_Position = ProjectionMatrix * ViewMatrix * meshModelMatrix * vec4(VertexPosition, 1);
	
	vertexPosition_ws = (meshModelMatrix * vec4(VertexPosition,1)).xyz;
	
	vec3 vertexPosition_cs = ( ViewMatrix * meshModelMatrix * vec4(VertexPosition,1)).xyz;

	normal_cs = ( ViewMatrix * meshModelMatrix * vec4(VertexNormal,0)).xyz;

	eyeDirection_cs = vec3(0,0,0) - vertexPosition_cs;

	normal_ws = (ModelMatrix * MeshMatrix * vec4(VertexNormal,0)).xyz;
	
	uv = VertexUV;

}



