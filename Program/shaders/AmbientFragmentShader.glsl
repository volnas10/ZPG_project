#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_NV_uniform_buffer_std430_layout : enable

#define PI 3.14159265359

in vec2 uv;

out vec4 color;

layout(std140, binding = 1) uniform Material {
	vec4 has_textures; // diffuse, specular, normal, rma
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 ambient_color;
	float roughness;
	float refraction_index;
	float shininess;
	float metallic;
	int texture_id;
} material;

struct TexturePack {
	uint64_t diffuse_texture;
	uint64_t specular_map;
	uint64_t normal_map;
	uint64_t rma_map;
};

layout(std430, binding = 2) uniform TexturePacks{
	TexturePack texturePacks[32];
};

void main(){
	// If material has texture, use it
	vec3 diffuseColor;
	if (material.has_textures[0] > uint(0)) {
		diffuseColor = texture(sampler2D(texturePacks[material.texture_id].diffuse_texture), uv).rgb;
	}
	else {
		diffuseColor = material.diffuse_color.rgb;
	}

	vec3 ambient_color = diffuseColor.rgb * 0.05;
	color.rgb = ambient_color;
	color.a = 1.0;
}