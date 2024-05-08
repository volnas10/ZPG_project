#version 450 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexUV;
layout(location = 3) in vec3 VertexTangent;
layout(location = 4) in vec3 VertexBitangent;

// ws - worldspace
// cs - cameraspace
// ms - modelspace
// ts - tangentspace

out vec2 uv;
out vec3 vertexPosition_ws;
out vec3 normal_cs;
out vec3 eyeDirection_cs;
out vec3 normal_ws;
flat out vec3 eyePosition_ws;
out mat3 TBN;
out vec3 lightDirection_ws;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionViewMatrix;

layout(std140, binding = 0) uniform ModelMatrices{
	mat4 model_matrix[1024];
};

uniform int LightCount;
layout(std140, binding = 3) uniform Light{
	vec4 position;
	vec4 direction;
	vec4 attenuation; // constant, linear, quadratic, padding
	mat4 lightspace_matrix;
	float angle_precalculated;
	uint type;
} Lights[6];

void main(){
	gl_Position = ProjectionViewMatrix * model_matrix[gl_InstanceID] * vec4(VertexPosition, 1);
	
	vertexPosition_ws = (model_matrix[gl_InstanceID] * vec4(VertexPosition, 1)).xyz;
	
	vec3 vertexPosition_cs = (ViewMatrix * vec4(vertexPosition_ws, 1)).xyz;

	mat4 modelViewMatrix = ViewMatrix * model_matrix[gl_InstanceID];

	mat4 normalMatrix = transpose(inverse(modelViewMatrix));

	normal_cs = normalize(vec3(normalMatrix * vec4(VertexNormal, 1)));

	eyeDirection_cs = normalize(vec3(0, 0, 0) - vertexPosition_cs);

	normal_ws = normalize(mat3(inverse(ViewMatrix)) * normal_cs);
	
	eyePosition_ws = vec3(inverse(ViewMatrix)[3]);

	vec3 T = normalize(mat3(model_matrix[gl_InstanceID]) * VertexTangent);
	vec3 B = normalize(mat3(model_matrix[gl_InstanceID]) * VertexBitangent);
	vec3 N = normalize(mat3(model_matrix[gl_InstanceID]) * VertexNormal);
	TBN = mat3(T, B, N);

	uv = VertexUV;

	if (LightCount > 0) {
		if (Lights[0].type == 1) {
			lightDirection_ws = normalize(-Lights[0].direction.xyz);
		}
		else {
			lightDirection_ws = normalize(Lights[0].position.xyz - vertexPosition_ws);
		}
	}
}



