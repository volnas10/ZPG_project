#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;
layout(location = 3) in int vertex_material;

// Output data ; will be interpolated for each fragment.
out int material_index;
out vec2 UV;
out vec3 vertex_position_worldspace;
out vec3 normal_cameraspace;
out vec3 eye_direction_cameraspace;
out vec3 light_direction_cameraspace;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat3 light_matrix;

void main(){
	material_index = vertex_material;

	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(vertex_position, 1);
	
	vertex_position_worldspace = (model_matrix * vec4(vertex_position,1)).xyz;
	
	vec3 vertex_position_cameraspace = ( view_matrix * model_matrix * vec4(vertex_position,1)).xyz;
	eye_direction_cameraspace = vec3(0,0,0) - vertex_position_cameraspace;

	vec3 light_position = light_matrix[0];
	vec3 light_position_cameraspace = ( view_matrix * vec4(light_position,1)).xyz;
	light_direction_cameraspace = light_position_cameraspace + eye_direction_cameraspace;
	
	normal_cameraspace = ( view_matrix * model_matrix * vec4(vertex_normal,0)).xyz;
	UV = vertex_uv;

}

