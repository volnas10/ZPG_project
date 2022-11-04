#version 460

in vec2 uv;
in vec3 vertexPosition_ws;
in vec3 vertexPosition_cs;
in vec3 normal_cs;
in vec3 eyeDirection_cs;
in vec3 lightDirections_cs[10];
in vec3 spotlightDirections_cs[10];

out vec4 color;

uniform sampler2D TextureSampler;
uniform mat3 LightMatrix;

layout(std140, binding=0) uniform Material {
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

uniform int LightCount;
layout(std140, binding=1, set=0) uniform Light {
	vec4 color;
	vec4 position;
	vec4 direction;
	vec4 attenuation; // constant, linear, quadratic, padding
	float angle;
	uint type;
} Lights[10];

vec3 pointLightCalculation(in mat3 colorParts, in vec3 lColor, in vec3 lPos, in vec3 att, in vec3 normal, in vec3 lDir, in vec3 eyeDir) {

	float diff = max(dot(normal, lDir), 0);
	float spec = 0;
	if (diff > 0) {
		vec3 R = reflect(-lDir, normal);
		spec = max(dot(eyeDir, R), 0);
	}

	float distance = length(lPos - vertexPosition_ws);
	float attenuation = 2.0 / ( att.x + att.y * distance + att.z * (distance * distance) );
	vec3 color = attenuation * ( colorParts[1] * lColor * diff +
								colorParts[2] * lColor * pow(spec, 20));

	return color;
}

vec3 directionalLightCalculation(in mat3 colorParts, in vec3 lColor, in vec3 normal, in vec3 lDir, in vec3 eyeDir) {

	float diff = max(dot(normal, -lDir), 0);
	float spec = 0;
	if (diff > 0) {
		vec3 R = reflect(lDir, normal);
		spec = max(dot(eyeDir, R), 0);
	}

	vec3 color =  colorParts[1] * lColor * diff +
				colorParts[2] * lColor * pow(spec, 20);

	return color;
}

vec3 spotlightCalculation(in mat3 colorParts, in vec3 lColor, in vec3 lPos, in vec3 att, in vec3 normal, in vec3 lDir, in vec3 lDir2, in vec3 eyeDir, in float angle) {
	float deviation = max(dot(-lDir, lDir2), 0);
	// Angle describes the whole light cone angle in degrees so ((angle / 2) * 3.14159) / 180 converts half of it to radians
	if (deviation < cos(angle * 0.03490658503)) return vec3(0);

	float diff = max(dot(normal, lDir), 0);
	float spec = 0;
	if (diff > 0) {
		vec3 R = reflect(-lDir, normal);
		spec = max(dot(eyeDir, R), 0);
	}

	float distance = length(lPos - vertexPosition_ws);
	float attenuation = 2.0 / ( att.x + att.y * distance + att.z * (distance * distance) );
	vec3 color = attenuation * ( colorParts[1] * lColor * diff +
								colorParts[2] * lColor * pow(spec, 20));

	return color;
}

// For flashlight eyeDirection is used as vector from vertex to light and vec3(0, 0, 1) is the direction of light from our camera
vec3 flashlightCalculation(in mat3 colorParts, in vec3 lColor, in vec3 att, in vec3 normal, in vec3 eyeDir, in float angle) {
	float limit = cos(angle * 0.00872663889);
	float deviation = max(dot(eyeDir, vec3(0, 0, 1)), 0);
	if (deviation < limit) return vec3(0);

	float diff = max(dot(normal, eyeDir), 0);
	float spec = 0;
	if (diff > 0) {
		vec3 R = reflect(-eyeDir, normal);
		spec = max(dot(eyeDir, R), 0);
	}

	float distance = length(vec3(0, 0, 0) - vertexPosition_cs);
	float attenuation = 3.0 / ( att.x + att.y * distance + att.z * (distance * distance));
	float circularAttenuation = sin((deviation - limit) * (1.5708 / (1 - limit)));
	vec3 color = attenuation * pow(circularAttenuation, 2) * ( colorParts[1] * lColor * diff +
								colorParts[2] * lColor * pow(spec, 20));

	return color;
}

void main(){

	// If material has texture, use it
	vec4 diffuseColor;
	vec3 ambientColor;
	vec3 specularColor;
	if (material.diffuse_texture > uint(0)) {
		diffuseColor = texture(TextureSampler, uv);
		ambientColor = vec3(0.2, 0.2, 0.2) * diffuseColor.rgb;
		specularColor = vec3(0.1,0.1,0.1);
	}
	else {
		diffuseColor = material.diffuse_color;
		ambientColor = material.ambient_color.rgb;
		// We don't want black ambient color
		if (ambientColor == vec3(0.0, 0.0, 0.0)) {
			ambientColor = vec3(0.2, 0.2, 0.2) * diffuseColor.rgb;
		}
		specularColor = material.specular_color.rgb;
	}

	vec3 N = normalize(normal_cs);
	vec3 E = normalize(eyeDirection_cs);
	vec3 finalColor = ambientColor;
	mat3 colorParts = mat3(ambientColor, diffuseColor.rgb, specularColor);
	for (int i = 0; i < LightCount; i++) {
		uint type = Lights[i].type;
		vec3 L = normalize(lightDirections_cs[i]);
		// Point light
		if (type == uint(0)) {
			finalColor += pointLightCalculation(colorParts, Lights[i].color.rgb, Lights[i].position.xyz, Lights[i].attenuation.xyz, N, L, E);
		}
		// Directional light
		else if (type == uint(1)) {
			finalColor += directionalLightCalculation(colorParts, Lights[i].color.rgb, N, L, E);
		}
		else if (type == uint(2)) {
			// L is direction from vertex to light and L2 direection of light
			vec3 L2 = normalize(spotlightDirections_cs[i]);
			finalColor += spotlightCalculation(colorParts, Lights[i].color.rgb, Lights[i].position.xyz, Lights[i].attenuation.xyz, N, L, L2, E, Lights[i].angle);
		}
		else if (type == uint(3)) {
			finalColor += flashlightCalculation(colorParts, Lights[i].color.rgb, Lights[i].attenuation.xyz, N, E, Lights[i].angle);
		}
	}

	
	color.rgb = finalColor;


	color.a = diffuseColor.a;

	if (color.a < 0.5) discard;
}