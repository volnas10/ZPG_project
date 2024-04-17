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
flat in vec3 eyePosition_ws;
in mat3 TBN;

out vec4 color;

uniform sampler2D IrradianceSampler;
uniform sampler2D PrefilteredMapSampler;
uniform sampler2D BRDFSampler;

uniform sampler2DArrayShadow DepthMaps;
uniform bool ShadowsOn;

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

uniform int LightCount;
layout(std140, binding = 3) uniform Light {
	vec4 position;
	vec4 direction;
	vec4 attenuation; // constant, linear, quadratic, padding
	mat4 lightspace_matrix;
	float angle_precalculated;
	uint type;
} Lights[6];

const vec2 shadowMultisample[5] = { vec2(0, 1), vec2(-1, 0), vec2(0, 0), vec2(1, 0), vec2(0, -1) };

void main(){
	vec3 normal = normalize(normal_ws);
	if (material.has_textures[1] > uint(0)) {
		normal = normalize(texture(sampler2D(texturePacks[material.texture_id].normal_map), uv).rgb * 2.0 - 1.0);
		normal = normalize(TBN * normal);
	}

	vec3 eye_direction = normalize(eyePosition_ws - vertexPosition_ws);
	float cos_theta = dot(normal_ws, eye_direction);
	if (cos_theta < 0) {
		normal = -normal;
		cos_theta = -cos_theta;
	}

	float u = atan(normal.z, normal.x) / (2 * PI);
	float v = acos(normal.y) / PI;

	// Sample irradiance map
	vec3 irradiance = texture(IrradianceSampler, vec2(u, 1 - v)).rgb;

	// If material has texture, use it
	vec3 diffuseColor;
	if (material.has_textures[0] > uint(0)) {
		diffuseColor = texture(sampler2D(texturePacks[material.texture_id].diffuse_texture), uv).rgb;
	}
	else {
		diffuseColor = material.diffuse_color.rgb;
	}
	vec3 specularColor;
	if (material.has_textures[1] > uint(0)) {
		specularColor = texture(sampler2D(texturePacks[material.texture_id].specular_map), uv).rgb;
	}
	else {
		specularColor = material.specular_color.rgb;
	}

	// Get shadowing
	float shadow = 1.0;
	if (ShadowsOn) {
		shadow = 0.0;
		for (int i = 0; i < LightCount; i++) {

			uint type = Lights[i].type;

			// Calculating coordinates in depth map
			vec4 vertexPosition_ls = Lights[i].lightspace_matrix * vec4(vertexPosition_ws, 1.0);
			vec3 fragmentPos = vertexPosition_ls.xyz / vertexPosition_ls.w;
			vec3 projCoords = fragmentPos * 0.5 + 0.5;
			projCoords.z -= 0.0004;

			vec2 texelSize = 1.0 / textureSize(DepthMaps, 0).xy;

			for (int j = 0; j < 5; j++)
			{
				shadow += 0.2 + texture(DepthMaps, vec4(projCoords.xy + shadowMultisample[j] * texelSize, i, projCoords.z)).r * 0.8;
			}
			shadow /= 5.0;
		}
	}
	float roughness = material.roughness;
	float metallic = material.metallic;
	float ao = 1.0;
	if (material.has_textures[3] > uint(0)) {
		vec3 rma = texture(sampler2D(texturePacks[material.texture_id].rma_map), uv).rgb;
		roughness = rma.r;
		metallic = rma.g;
		ao = rma.b;
	}

	vec2 brdf = texture(BRDFSampler, vec2(cos_theta, roughness)).rg;
	vec3 F0 = mix(vec3(0.04), diffuseColor, metallic);
	vec3 F = F0 + (1 - F0) * pow(2, (-5.55473 / cos_theta - 6.98316) * cos_theta);
	vec3 Fd = (vec3(1.0) - F) * (1 - metallic);

	vec3 diffuse_part = Fd * (diffuseColor / PI) * irradiance;
	vec3 reflected = reflect(-eye_direction, normal);
	float ref_u = atan(reflected.z, reflected.x) / (2 * PI);
	float ref_v = acos(reflected.y) / PI;
	vec3 specular_part = textureLod(PrefilteredMapSampler, vec2(ref_u, 1 - ref_v), 6.0 * roughness).rgb;

	vec3 ambientColor = diffuseColor.rgb * 0.05;
	color.rgb = diffuse_part * shadow + (F0 * brdf.r + brdf.g) * specular_part * ao;
	color.a = 1.0;
}