#version 330

layout(location=0) in vec3 vp;
layout(location=1) in vec3 vertex_color;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 frag_color;

void main () {
		gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(vp, 1.0);
		frag_color = vec3(1.0, 1.0, 1.0) - vertex_color;
}