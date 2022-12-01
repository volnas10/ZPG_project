#version 460

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexUV;
  
out vec2 uv;

uniform float AspectRatio;

void main()
{
    gl_Position = vec4(VertexPosition.x / AspectRatio, VertexPosition.y, -1, 1);
    uv = VertexUV;
}