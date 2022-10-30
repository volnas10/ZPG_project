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
	vec4 diffuse_color; // r, g, b | texture_index
	vec4 specular_color;
	vec4 ambient_color;
	vec4 emissive_color;
	vec4 transparent_color; // r, g, b | opacity
	vec4 reflective_color; // r, g, b | reflectivity
	float refraction_index;
	float shininess;
	float shininess_strength;
} material;

void main(){

	vec3 lightColor = vec3(LightMatrix[1].x, LightMatrix[1].y, LightMatrix[1].z);
	float lightPower = LightMatrix[2].x * 0.2; // Reduced light power for sun
	vec3 lightPosition = LightMatrix[0];

	// If material has texture, use it
	vec3 diffuseColor;
	vec3 ambientColor;
	vec3 specularColor;
	if (material.diffuse_color.w >= 0) {
		diffuseColor = texture(TextureSampler, uv).rgb;
		ambientColor = vec3(0.1,0.1,0.1) * diffuseColor;
		specularColor = vec3(0.1,0.1,0.1);
	}
	else {
		diffuseColor = material.diffuse_color.rgb;
		ambientColor = material.ambient_color.rgb;
		specularColor = material.specular_color.rgb;
	}

	// Distance to the light
	float distance = length(lightPosition - vertexPosition_ws);
	// Lambert
	vec3 N = normalize(-normal_cs); // Normal inverted so sun glows outside
	vec3 L = normalize(lightDirection_cs);
	float cosTheta = clamp(dot(N, L), 0, 1);
	
	// Phong
	float cosAlpha = clamp(dot(normalize(eyeDirection_cs), reflect(-L, N)), 0, 1);
	
	color.rgb = ambientColor +
			diffuseColor * lightColor * lightPower * cosTheta / (distance * distance) +
			specularColor * lightColor * lightPower * pow(cosAlpha, material.shininess) / (distance * distance);
	
	color.a = material.transparent_color.a;
}