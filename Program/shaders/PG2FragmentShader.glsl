#version 330 core

in vec2 uv;
in vec3 vertexPosition_ws;
in vec3 normal_cs;
in vec3 eyeDirection_cs;
in vec3 normal_ws;

out vec4 color;

uniform sampler2D IrradianceSampler;
uniform sampler2D TextureSampler;

layout(std140) uniform Material {
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 ambient_color;
	vec4 emissive_color;
	vec4 transparent_color; // r, g, b | opacity
	vec4 reflective_color; // r, g, b | reflectivity
	float refraction_index;
	float shininess;
	float shininess_strength;
	uint diffuse_texture;
	uint normal_map;
	uint opacity_map;
} material;

void main(){

	float ir_u = 0.5 + atan(normal_ws.z, normal_ws.x) / (2 * 3.14159265359);
	float ir_v = 0.5 - asin(normal_ws.y) / 3.14159265359;
	vec3 irradiance = texture(IrradianceSampler, vec2(ir_u, 1 - ir_v)).rgb;

	// If material has texture, use it
	vec4 diffuseColor;
	if (material.diffuse_texture > uint(0)) {
		diffuseColor = texture(TextureSampler, uv);
	}
	else {
		diffuseColor = material.diffuse_color;
	}
	
	color.rgb = material.ambient_olor + diffuseColor.rgb * irradiance;
	color.a = diffuseColor.a;
}