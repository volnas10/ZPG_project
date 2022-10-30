#version 330 core

in vec2 uv;
in vec3 eyeDirection_cs;
in vec3 lightDirection_cs;
in vec3 normal_cs;

uniform sampler2D DiffuseSampler;
uniform sampler2D NormalSampler;
uniform sampler2D SpecularSampler;

uniform vec2 HasTextures;
uniform mat3 LightMatrix;

out vec4 color;

void main() {
	vec3 diffuseColor = texture(DiffuseSampler, uv).rgb;
	vec3 ambientColor = vec3(LightMatrix[2].y) * diffuseColor;

	// Without normal map, plane will be flat
    vec3 normal = vec3(0.0, 1.0, 0.0);
    if (HasTextures.x > 0) {
        normal = texture(DiffuseSampler, uv).rgb;
    }

	vec3 specularColor = vec3(0.1);
	if (HasTextures.y > 0) {
		specularColor = texture(SpecularSampler, uv).rgb;
	}


	color.rgb = diffuseColor;
	color.a = 1.0;

}