#version 330 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexUV;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;

// ws - worldspace
// cs - cameraspace
// ms - modelspace
// ts - tangentspace

out vec2 uv;
out vec3 vertexPosition_ws;
out vec3 normal_cs;
out vec3 eyeDirection_cs;
out vec3 lightDirection_cs;

out vec3 eyeDirection_ts;
out vec3 lightDirection_ts;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 MeshMatrix;
// Contains position, color and power
uniform mat3 LightMatrix;


out vec3 testNormal;

void main(){
	mat4 meshModelMatrix = ModelMatrix * MeshMatrix;
	mat3 MMV3x3 = mat3(ViewMatrix * meshModelMatrix);

	gl_Position = ProjectionMatrix * ViewMatrix * meshModelMatrix * vec4(VertexPosition, 1);
	
	vertexPosition_ws = (meshModelMatrix * vec4(VertexPosition,1)).xyz;
	
	vec3 vertexPosition_cs = MMV3x3 * VertexPosition;

	normal_cs = MMV3x3 * normalize(VertexNormal);
	vec3 tangent_cs = MMV3x3 * normalize(Tangent);
	//tangent_cs = normalize(tangent_cs - dot(tangent_cs, normal_cs) * normal_cs);


	vec3 bitangent_cs = MMV3x3 * normalize(Bitangent);
	//bitangent_cs = normalize(bitangent_cs - dot(bitangent_cs, normal_cs) * normal_cs);
	
	mat3 TBN = transpose(mat3(tangent_cs, bitangent_cs, normal_cs));

	eyeDirection_cs = vec3(0,0,0) - vertexPosition_cs;
	eyeDirection_ts = TBN * eyeDirection_cs;

	vec3 lightPosition_cs = ( ViewMatrix * vec4(LightMatrix[0],1)).xyz;
	lightDirection_cs = lightPosition_cs + eyeDirection_cs;
	lightDirection_ts = TBN * lightDirection_cs;
	
	uv = VertexUV;

}

