#version 460 core

layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 18) out;

uniform int LightCount;
layout(std140, binding = 3) uniform Light {
	vec4 position;
	vec4 direction;
	vec4 attenuation; // constant, linear, quadratic, padding
	mat4 lightspace_matrix;
	float angle_precalculated;
	uint type;
} Lights[6];

uniform mat4 ProjectionViewMatrix;

void EmitSillhouette(vec3 v0, vec3 v1, vec3 v0_dir, vec3 v1_dir) {
	gl_Position = ProjectionViewMatrix * vec4(v0 - v0_dir * 0.001, 1.0);
	EmitVertex();
	gl_Position = ProjectionViewMatrix * vec4(-v0_dir, 0.0);
	EmitVertex();
	gl_Position = ProjectionViewMatrix * vec4(v1 - v1_dir * 0.001, 1.0);
	EmitVertex();
	gl_Position = ProjectionViewMatrix * vec4(-v1_dir, 0.0);
	EmitVertex();

	EndPrimitive();
}

void EmitSillhouette_lines(vec3 v0, vec3 v1, vec3 v0_dir, vec3 v1_dir) {
	gl_Position = ProjectionViewMatrix * vec4(v0 - v0_dir * 0.0001, 1.0);
	EmitVertex();
	gl_Position = ProjectionViewMatrix * vec4(-v0_dir, 0.0);
	EmitVertex();
	gl_Position = ProjectionViewMatrix * vec4(-v1_dir, 0.0);
	EmitVertex();
	gl_Position = ProjectionViewMatrix * vec4(v1 - v1_dir * 0.001, 1.0);
	EmitVertex();
	gl_Position = ProjectionViewMatrix * vec4(v0 - v0_dir * 0.001, 1.0);
	EmitVertex();

	EndPrimitive();
}

void main() {
	// Main triangle
	const vec3 v0 = gl_in[0].gl_Position.xyz;
	const vec3 v1 = gl_in[4].gl_Position.xyz;
	const vec3 v2 = gl_in[2].gl_Position.xyz;

	const vec3 v0_side = gl_in[5].gl_Position.xyz;
	const vec3 v1_side = gl_in[3].gl_Position.xyz;
	const vec3 v2_side = gl_in[1].gl_Position.xyz;

	vec3 main_normal = normalize(cross(v1 - v0, v2 - v0));
	vec3 light_dir0;
	vec3 light_dir1;
	vec3 light_dir2;
	if (Lights[0].type == 1) {
		light_dir0 = -Lights[0].direction.xyz;
		light_dir1 = -Lights[0].direction.xyz;
		light_dir2 = -Lights[0].direction.xyz;
	}
	else {
		light_dir0 = normalize(Lights[0].position.xyz - v0);
		light_dir1 = normalize(Lights[0].position.xyz - v1);
		light_dir2 = normalize(Lights[0].position.xyz - v2);
	}


	// If triangle is facing the light
	if (dot(main_normal, light_dir0) > 0.0) {

		//EmitSillhouette(v0, v1, light_dir0, light_dir1);
		//EmitSillhouette(v1, v2, light_dir1, light_dir2);
		//EmitSillhouette(v2, v0, light_dir2, light_dir0);

		
		// First side triangle
		vec3 normal = normalize(cross(v0_side - v0, v1 - v0));
		if (dot(normal, light_dir0) < 0.0) {
			EmitSillhouette(v0, v1, light_dir0, light_dir1);
		}

		// Second side triangle
		
		normal = normalize(cross(v1_side - v1, v2 - v1));
		if (dot(normal, light_dir1) < 0.0) {
			EmitSillhouette(v1, v2, light_dir1, light_dir2);
		}

		// Third side triangle
		normal = normalize(cross(v2_side - v2, v0 - v2));
		if (dot(normal, light_dir2) < 0.0) {
			EmitSillhouette(v2, v0, light_dir2, light_dir0);
		}
		
		

		// Front cap
		
		gl_Position = ProjectionViewMatrix * vec4(v0 - light_dir0 * 0.001, 1.0);
		EmitVertex();
		gl_Position = ProjectionViewMatrix * vec4(v1 - light_dir1 * 0.001, 1.0);
		EmitVertex();
		gl_Position = ProjectionViewMatrix * vec4(v2 - light_dir2 * 0.001, 1.0);
		EmitVertex();
		EndPrimitive();

		// Back cap
		gl_Position = ProjectionViewMatrix * vec4(-light_dir0, 0.0);
		EmitVertex();
		gl_Position = ProjectionViewMatrix * vec4(-light_dir1, 0.0);
		EmitVertex();
		gl_Position = ProjectionViewMatrix * vec4(-light_dir2, 0.0);
		EmitVertex();
		EndPrimitive();
		
	}

}
