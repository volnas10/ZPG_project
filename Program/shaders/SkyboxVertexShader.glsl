#version 330 core
layout (location = 0) in vec3 VertexPosition;

out vec3 uv;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

void main()
{
    gl_Position = ProjectionMatrix * mat4(mat3(ViewMatrix)) * vec4(VertexPosition, 1.0);
    uv = VertexPosition;
}  