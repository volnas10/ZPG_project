#version 460

in vec2 uv;
in vec3 vertexPosition_ws;
in vec3 vertexPosition_cs;
in vec3 normal_cs;
in vec3 eyeDirection_cs;
in vec3 lightDirections_cs[8];
in vec3 spotlightDirections_cs[8];
in float lightDistances[8];

in vec3 eyeDirection_ts;
in vec3 lightDirection_ts;

out vec4 color;

uniform sampler2D DiffuseTextureSampler;
uniform sampler2D NormalTextureSampler;
uniform sampler2D OpacityTextureSampler;
uniform vec3 HasTextures;

uniform sampler2DArrayShadow DepthMaps;
uniform bool ShadowsOn;

layout(std140, binding=1) uniform Material {
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 ambient_color;
	vec4 emissive_color;
	vec4 transparent_color; // r, g, b | opacity
	vec4 reflective_color; // r, g, b | reflectivity
	float refraction_index;
	float shininess;
	float shininess_strength;
} material;

uniform int LightCount;
layout(std140, binding=2) uniform Light {
	vec4 color;
	vec4 position;
	vec4 direction;
	vec4 attenuation; // constant, linear, quadratic, padding
	mat4 lightspace_matrix;
	float angle_precalculated;
	uint type;
} Lights[6];

vec3 pointLightCalculation(in mat3 colorParts, in int lightIndex, in vec3 normal, in vec3 lDir, in vec3 eyeDir) {

	float diff = max(dot(normal, lDir), 0);
	float spec = 0;
	if (diff > 0) {
		vec3 R = reflect(-lDir, normal);
		spec = max(dot(eyeDir, R), 0);
	}
	vec3 att = Lights[lightIndex].attenuation.xyz;
	float distance = lightDistances[lightIndex];
	float attenuation = 2.0 / ( att.x + att.y * distance + att.z * (distance * distance));
	vec3 color = attenuation * ( colorParts[1] * Lights[lightIndex].color.rgb * diff +
								colorParts[2] * Lights[lightIndex].color.rgb * pow(spec, 20));

	return color;
}

vec3 directionalLightCalculation(in mat3 colorParts, in int lightIndex, in vec3 normal, in vec3 lDir, in vec3 eyeDir) {

	float diff = max(dot(normal, -lDir), 0);
	float spec = 0;
	if (diff > 0) {
		vec3 R = reflect(lDir, normal);
		spec = max(dot(eyeDir, R), 0);
	}

	vec3 color =  colorParts[1] * Lights[lightIndex].color.rgb * diff +
				colorParts[2] * Lights[lightIndex].color.rgb * pow(spec, 20);

	return color;
}

vec3 spotlightCalculation(in mat3 colorParts, in int lightIndex, in vec3 normal, in vec3 lDir, in vec3 lDir2, in vec3 eyeDir) {
	float deviation = max(dot(-lDir, lDir2), 0);
	// Angle describes the whole light cone angle in degrees so ((angle / 2) * 3.14159) / 180 converts half of it to radians
	if (deviation < Lights[lightIndex].angle_precalculated) return vec3(0);

	float diff = max(dot(normal, lDir), 0);
	float spec = 0;
	if (diff > 0) {
		vec3 R = reflect(-lDir, normal);
		spec = max(dot(eyeDir, R), 0);
	}

	vec3 att = Lights[lightIndex].attenuation.xyz;
	float distance = lightDistances[lightIndex];
	float attenuation = 2.0 / ( att.x + att.y * distance + att.z * (distance * distance) );
	vec3 color = attenuation * ( colorParts[1] * Lights[lightIndex].color.rgb * diff +
								colorParts[2] * Lights[lightIndex].color.rgb * pow(spec, 20));

	return color;
}

// For flashlight eyeDirection is used as vector from vertex to light and vec3(0, 0, 1) is the direction of light from our camera
vec3 flashlightCalculation(in mat3 colorParts, in int lightIndex, in vec3 normal, in vec3 eyeDir) {
	float deviation = max(dot(eyeDir, vec3(0, 0, 1)), 0);
	if (deviation < Lights[lightIndex].angle_precalculated) return vec3(0);

	float diff = max(dot(normal, eyeDir), 0);
	float spec = 0;
	if (diff > 0) {
		vec3 R = reflect(-eyeDir, normal);
		spec = max(dot(eyeDir, R), 0);
	}

	vec3 att = Lights[lightIndex].attenuation.xyz;
	float distance = length(vec3(0, 0, 0) - vertexPosition_cs);
	float attenuation = 3.0 / ( att.x + att.y * distance + att.z * (distance * distance));
	float circularAttenuation = sin((deviation - Lights[lightIndex].angle_precalculated) * (1.5708 / (1 - Lights[lightIndex].angle_precalculated)));
	vec3 color = attenuation * pow(circularAttenuation, 2) * ( colorParts[1] * Lights[lightIndex].color.rgb * diff +
								colorParts[2] * Lights[lightIndex].color.rgb * pow(spec, 20));


	return color;
}

const vec2 shadowMultisample[5] = {vec2(0, 1), vec2(-1, 0), vec2(0, 0), vec2(1, 0), vec2(0, -1)};

void main(){
	// If material has texture, use it
	vec4 diffuseColor;
	vec3 ambientColor;
	vec3 specularColor;
	if (HasTextures.x > 0) {
		diffuseColor = texture(DiffuseTextureSampler, uv);
		ambientColor = vec3(0.2, 0.2, 0.2) * diffuseColor.rgb;
		specularColor = vec3(0.3,0.3,0.3);
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

	// Set transparency
	if (HasTextures.z > 0) {
		color.a = texture(OpacityTextureSampler, uv).r;
	}
	else {
		color.a = diffuseColor.a;
	}

	if (color.a < 0.8) discard;

	vec3 N = normalize(normal_cs);
	vec3 E = normalize(eyeDirection_cs);
	vec3 finalColor = ambientColor;
	mat3 colorParts = mat3(ambientColor, diffuseColor.rgb, specularColor);

	for (int i = 0; i < LightCount; i++) {

		uint type = Lights[i].type;
		vec3 L = normalize(lightDirections_cs[i]);

		// Calculating coordinates in depth map
		vec4 vertexPosition_ls = Lights[i].lightspace_matrix * vec4(vertexPosition_ws, 1.0);
		vec3 fragmentPos = vertexPosition_ls.xyz / vertexPosition_ls.w;
		vec3 projCoords = fragmentPos * 0.5 + 0.5;
		projCoords.z -= 0.005;

		float shadow = 0.0;
		if (ShadowsOn) {
			vec2 texelSize = 1.0 / textureSize(DepthMaps, 0).xy;

			for(int j = 0; j < 5; j++)
			{
				shadow += texture(DepthMaps, vec4(projCoords.xy + shadowMultisample[j] * texelSize, i, projCoords.z)).r;
			}
			shadow /= 5.0;
		}
		else {
			shadow = 1.0;
		}


		// Point light
		if (type == uint(0)) {
			finalColor += (pointLightCalculation(colorParts, i, N, L, E) * shadow);
		}
		// Directional light
		else if (type == uint(1)) {
			finalColor += (directionalLightCalculation(colorParts, i, N, L, E) * shadow);
		}
		else if (type == uint(2)) {
			// L is direction from vertex to light and L2 direection of light
			vec3 L2 = normalize(spotlightDirections_cs[i]);
			finalColor += (spotlightCalculation(colorParts, i, N, L, L2, E) * shadow);
		}
		else if (type == uint(3)) {
			finalColor += (flashlightCalculation(colorParts, i, N, E) * shadow);
		}
	}

	color.rgb = finalColor;

}