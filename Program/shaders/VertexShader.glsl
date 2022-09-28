#version 330

layout(location=0) in vec3 vp;

uniform mat4 MVP;

out vec3 frag_color;

void main () {
		gl_Position = MVP * vec4(vp, 1.0);
		frag_color = vp;
}