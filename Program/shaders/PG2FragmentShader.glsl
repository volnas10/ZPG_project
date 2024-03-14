#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_NV_uniform_buffer_std430_layout : enable

#define PI 3.14159265359

in vec2 uv;
in vec3 vertexPosition_ws;
in vec3 normal_cs;
in vec3 eyeDirection_cs;
in vec3 normal_ws;

out vec4 color;

uniform sampler2D IrradianceSampler;

uniform sampler2DArrayShadow DepthMaps;
uniform bool ShadowsOn;

layout(std140, binding = 1) uniform Material {
	vec4 has_textures; // diffuse, specular, normal
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 ambient_color;
	float refraction_index;
	float shininess;
	int texture_id;
} material;

struct TexturePack {
	uint64_t diffuse_texture;
	uint64_t specular_map;
	uint64_t normal_map;
	uint64_t brdf_integration_map;
};

layout(std430, binding = 2) uniform TexturePacks{
	TexturePack texturePacks[32];
};

void main(){

	float u = 0.5 + atan(normal_ws.z, normal_ws.x) / (2 * PI);
	float v = 0.5 - asin(normal_ws.y) / PI;

	// Sample HDR texture
	vec3 irradiance = texture(IrradianceSampler, vec2(u, 1 - v)).rgb;

	// If material has texture, use it
	vec4 diffuseColor;
	if (material.has_textures[0] > uint(0)) {
		diffuseColor = texture(sampler2D(texturePacks[material.texture_id].diffuse_texture), uv);
	}
	else {
		diffuseColor = material.diffuse_color;
	}
	vec3 ambientColor = diffuseColor.rgb * 0.05;
	color.rgb = ambientColor + diffuseColor.rgb * irradiance * 0.95;
	color.a = diffuseColor.a;

	color.a = 1.0;
}