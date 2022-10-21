#version 330 core
layout (location = 0) in vec3 vertex;

out vec3 uv;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

void main()
{
    gl_Position = projection_matrix * mat4(mat3(view_matrix)) * vec4(vertex, 1.0);
    uv = vertex;
}  