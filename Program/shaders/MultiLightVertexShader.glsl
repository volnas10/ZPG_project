#version 460

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexUV;

// ws - worldspace
// cs - cameraspace
// ms - modelspace
// ts - tangentspace

out vec2 uv;
out vec3 vertexPosition_ws;
out vec3 vertexPosition_cs;
out vec3 normal_cs;
out vec3 eyeDirection_cs;
out vec3 lightDirections_cs[10];
out vec3 spotlightDirections_cs[10];

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 MeshMatrix;

uniform int LightCount;
layout(std140, binding=1) uniform Light {
	vec4 color;
	vec4 position;
	vec4 direction;
	vec4 attenuation; // constant, linear, quadratic, padding
	float angle;
	uint type;
} Lights[10];

void main(){
	mat4 meshModelMatrix = ModelMatrix * MeshMatrix;

	gl_Position = ProjectionMatrix * ViewMatrix * meshModelMatrix * vec4(VertexPosition, 1);
	
	vertexPosition_ws = (meshModelMatrix * vec4(VertexPosition,1)).xyz;
	
	vertexPosition_cs = ( ViewMatrix * meshModelMatrix * vec4(VertexPosition,1)).xyz;

	normal_cs = ( ViewMatrix * meshModelMatrix * vec4(VertexNormal,0)).xyz;

	eyeDirection_cs = vec3(0,0,0) - vertexPosition_cs;

	for (int i = 0; i < LightCount; i++) {
		uint type = Lights[i].type;
		// Point light
		vec3 dirToLight;
		if (type == uint(0)) {
			vec3 lightPosition_cs = ( ViewMatrix * vec4(Lights[i].position.xyz, 1)).xyz;
			lightDirections_cs[i] = lightPosition_cs + eyeDirection_cs;
		}
		// Directional light
		else if (type == uint(1)) {
			lightDirections_cs[i] = ( ViewMatrix * vec4(Lights[i].direction.xyz, 0)).xyz;
		}
		// Spotlight
		else if (type == uint(2)) {
			vec3 lightPosition_cs = ( ViewMatrix * vec4(Lights[i].position.xyz, 1)).xyz;
			lightDirections_cs[i] = lightPosition_cs + eyeDirection_cs;
			spotlightDirections_cs[i] = ( ViewMatrix * vec4(Lights[i].direction.xyz, 0)).xyz;
		}
	}

	uv = VertexUV;

}



