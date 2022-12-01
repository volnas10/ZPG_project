#version 460

uniform sampler2D TextureSampler;
uniform sampler2D OpacitySampler;
uniform vec2 HasTextures;

in vec2 uv;

out vec4 color;

void main() {

	color = vec4(1.0);


	if (HasTextures.y > 0) {
		color.a = texture(OpacitySampler, uv).r;
	}

	if (color.a < 0.7) discard;

	if (HasTextures.x > 0) {
		color.a = texture(TextureSampler, uv).a;
	}

	if (color.a < 0.7) discard;
}