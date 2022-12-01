#version 460
out vec4 color;
  
in vec2 uv;

uniform sampler2D Crosshair;
uniform float AspectRatio;

void main()
{             
    color = texture(Crosshair, uv);
}