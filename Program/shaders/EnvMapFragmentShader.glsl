#version 460 core
out vec4 color;

in vec3 fragDirection;

uniform sampler2D Skybox;

void main()
{    
    vec3 dir = normalize(fragDirection);

    // Convert direction vector to UV coordinates
    
    float u = 0.5 + atan(dir.z, dir.x) / (2 * 3.14159265359);
	float v = 0.5 - asin(dir.y) / 3.14159265359;

    // Sample HDR texture
    color = texture(Skybox, vec2(u, 1 - v));
    //color = vec4(dir, 1.0);
}