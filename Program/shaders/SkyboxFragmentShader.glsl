#version 330 core
out vec4 color;

in vec3 uv;

uniform samplerCube Skybox;

void main()
{    
    color = texture(Skybox, -uv);
}