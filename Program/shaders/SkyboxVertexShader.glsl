#version 330 core
layout (location = 0) in vec3 VertexPosition;

out vec3 uv;

uniform mat4 ProjectionViewMatrix;
uniform mat4 ViewMatrix;

void main()
{
    mat4 projectionMatrix = ProjectionViewMatrix * inverse(ViewMatrix);
    gl_Position = projectionMatrix * mat4(mat3(ViewMatrix)) * vec4(VertexPosition, 1.0);
    uv = VertexPosition;
}  