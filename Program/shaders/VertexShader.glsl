#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 vertex_position_worldspace;
out vec3 normal_cameraspace;
out vec3 eye_direction_cameraspace;
out vec3 light_direction_cameraspace;

// Values that stay constant for the whole mesh.
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat3 light_matrix;

void main(){
	
	gl_Position =  projection_matrix * view_matrix * model_matrix * vec4(vertex_position, 1);
	
	// Position of the vertex, in worldspace : M * position
	vertex_position_worldspace = (model_matrix * vec4(vertex_position,1)).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertex_position_cameraspace = ( view_matrix * model_matrix * vec4(vertex_position,1)).xyz;
	eye_direction_cameraspace = vec3(0,0,0) - vertex_position_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 light_position = light_matrix[0];
	vec3 light_position_cameraspace = ( view_matrix * vec4(light_position,1)).xyz;
	light_direction_cameraspace = light_position_cameraspace + eye_direction_cameraspace;
	
	// Normal of the the vertex, in camera space
	normal_cameraspace = ( view_matrix * model_matrix * vec4(vertex_normal,0)).xyz;
	UV = vertex_uv;

}

