#version 330 core

in vec2 uv;
in vec3 vertexPosition_ws;
in vec3 normal_cs;
in vec3 eyeDirection_cs;
in vec3 lightDirection_cs;

out vec4 color;

uniform sampler2D TextureSampler;
uniform mat3 LightMatrix;

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

	vec3 lightColor = vec3(LightMatrix[1].x, LightMatrix[1].y, LightMatrix[1].z);
	float lightPower = LightMatrix[2].x;
	vec3 lightPosition = LightMatrix[0];
	float ambient_light = LightMatrix[2].y;

	// If material has texture, use it
	vec4 diffuseColor;
	vec3 ambientColor;
	vec3 specularColor;
	if (material.diffuse_texture > uint(0)) {
		diffuseColor = texture(TextureSampler, uv);
		ambientColor = vec3(ambient_light) * diffuseColor.rgb;
		specularColor = vec3(0.1,0.1,0.1);
	}
	else {
		diffuseColor = material.diffuse_color;
		ambientColor = material.ambient_color.rgb;
		// We don't want black ambient color
		if (ambientColor == vec3(0.0, 0.0, 0.0)) {
			ambientColor = vec3(ambient_light) * diffuseColor.rgb;
		}
		specularColor = material.specular_color.rgb;
	}

	// Distance to the light
	float distance = length(lightPosition - vertexPosition_ws);

	vec3 N = normalize(normal_cs);
	vec3 L = normalize(lightDirection_cs);
	vec3 E = normalize(eyeDirection_cs);


	// Lambert
	float cosAlpha = max(dot(N, L), 0);
	
	// Phong
	vec3 R = reflect(-L, N);
	float cosTheta = 0;
	if (cosAlpha > 0) {
		cosTheta = max(dot(E, R), 0);
	}
	
	color.rgb = ambientColor +
			diffuseColor.rgb * lightColor * lightPower * cosAlpha / (distance * distance) +
			specularColor * lightColor * lightPower * pow(cosTheta, 5) / (distance * distance);

	// Set transparency

	color.a = diffuseColor.a;

	if (color.a < 0.5) discard;
}