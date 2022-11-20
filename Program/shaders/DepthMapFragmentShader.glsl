#version 460
out vec4 color;
  
in vec2 uv;

uniform sampler2DArrayShadow DepthMap;

void main()
{             
    float depthValue = texture(DepthMap, vec4(uv, 0, 0.0)).r;
    color = vec4(vec3(depthValue), 1.0);
} 
