#version 460 core

layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 6) out;

uniform int LightCount;
layout(std140, binding = 3) uniform Light{
	vec4 position;
	vec4 direction;
	vec4 attenuation; // constant, linear, quadratic, padding
	mat4 lightspace_matrix;
	float angle_precalculated;
	uint type;
} Lights[6];

uniform mat4 PVmatrix;

void main() {
	// built-in input variable (array) gl_in[].gl_Position is expected to contain world-space coordinates of triangle (and adjacent) vertices
	// in this example, GL_TRIANGLES_ADJACENCY is expected
	const vec3 v0 = gl_in[0].gl_Position.xyz;
	const vec3 v1 = gl_in[2].gl_Position.xyz;
	const vec3 v2 = gl_in[4].gl_Position.xyz;

	// generate the first triangle
	gl_Position = PVmatrix * v0; // we should end up with the clip-space output position of the current vertex here
	EmitVertex();
	gl_Position = PVmatrix * v1;
	EmitVertex();
	gl_Position = PVmatrix * v2;
	EmitVertex();
	EndPrimitive();

	// generate the second triangle 10 units above the first one
	gl_Position = PVmatrix * (v0 + vec4(0.0f, 0.0f, 10.0f, 0.0f));
	EmitVertex();
	gl_Position = PVmatrix * (v1 + vec4(0.0f, 0.0f, 10.0f, 0.0f));
	EmitVertex();
	gl_Position = PVmatrix * (v2 + vec4(0.0f, 0.0f, 10.0f, 0.0f));
	EmitVertex();
	EndPrimitive();
}