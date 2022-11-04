#version 330 core

in vec2 uv;
in vec3 eyeDirection_cs;
in vec3 lightDirection_cs;
in vec3 normal_cs;

uniform sampler2D TextureSampler;

uniform mat3 LightMatrix;

out vec4 color;

void main() {
	vec3 lightColor = vec3(LightMatrix[1].x, LightMatrix[1].y, LightMatrix[1].z);
	float lightPower = LightMatrix[2].x;

	vec3 diffuseColor = texture(TextureSampler, uv).rgb;
	vec3 ambientColor = vec3(LightMatrix[2].y) * diffuseColor;

	vec3 N = normalize(normal_cs);

	vec3 specularColor = vec3(0.1);

	vec3 L = normalize(lightDirection_cs);
	float cosAlpha = max(dot(L, N), 0);

	vec3 E = normalize(eyeDirection_cs);
	vec3 R = normalize(reflect(-L, N));

	float cosTheta = 0;
	if (cosAlpha > 0) {
		cosTheta = max(dot(E, R), 0);
	}

	color.rgb = ambientColor + 
				diffuseColor * lightColor * lightPower * cosAlpha +
				specularColor * lightColor * lightPower * pow(cosTheta, 5);

	color.a = 1.0;

}