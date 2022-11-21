#version 330 core
out vec4 color;

in vec3 uv;

uniform samplerCube Skybox;

void main()
{    
    color = texture(Skybox, vec3(uv.x, -uv.y, uv.z));
}