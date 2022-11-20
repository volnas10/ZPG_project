#version 460

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexOffset;
layout(location = 2) in float TileRotation;

// ws - worldspace
// cs - cameraspace
// ms - modelspace
// ts - tangentspace

out vec2 uv;
out vec3 eyeDirection_cs;
out vec3 lightDirection_cs;
out vec3 normal_cs;
out vec3 vertexPosition_ws;
out vec3 vertexPosition_cs;
out vec3 lightDirections_cs[10];
out vec3 spotlightDirections_cs[10];

uniform mat4 ViewMatrix;
uniform mat4 ProjectionViewMatrix;

uniform int LightCount;
layout(std140, binding=2) uniform Light {
	vec4 color;
	vec4 position;
	vec4 direction;
	vec4 attenuation; // constant, linear, quadratic, padding
	float angle_precalculated;
	uint type;
} Lights[10];

mat3 rotate90 = mat3(0, 0, -1, 0, 1, 0, 1, 0, 0);
mat3 rotate180 = mat3(-1, 0, 0, 0, 1, 0, 0, 0, -1);
mat3 rotate270 = mat3(0, 0, 1, 0, 1, 0, -1, 0, 0);

void main(){

	vec3 vertexPosition;
    if (TileRotation == 0.0) { vertexPosition = VertexPosition;}
    else if (TileRotation == 1.0) {vertexPosition = rotate90 * VertexPosition;}
    else if (TileRotation == 2.0) {vertexPosition = rotate180 * VertexPosition;}
    else vertexPosition = rotate270 * VertexPosition;

    vertexPosition_ws = vec3(vertexPosition.x + VertexOffset.y, vertexPosition.y, vertexPosition.z + VertexOffset.x);
    gl_Position = ProjectionViewMatrix * vec4(vertexPosition_ws, 1.0);
    uv = vec2(0);
    if (VertexPosition.x > 0) uv.x = 1.0;
    if (VertexPosition.z > 0) uv.y = 1.0;
	
	vertexPosition_cs = ( ViewMatrix * vec4(vertexPosition_ws,1)).xyz;
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

	normal_cs = (ViewMatrix * vec4(0, 1, 0, 0)).xyz;

}