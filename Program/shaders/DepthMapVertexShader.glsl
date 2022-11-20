#version 460

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexUV;
  
out vec2 uv;

void main()
{
    gl_Position = vec4(VertexPosition, 1.0);
    uv = VertexUV;
}
