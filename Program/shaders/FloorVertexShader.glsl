#version 330 core
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexOffset;
layout (location = 2) in float TileRotation;

out vec2 uv;
out vec3 eyeDirection_cs;
out vec3 lightDirection_cs;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat3 LightMatrix;

mat3 rotate90 = mat3(0, 0, -1, 0, 1, 0, 1, 0, 0);
mat3 rotate180 = mat3(-1, 0, 0, 0, 1, 0, 0, 0, -1);
mat3 rotate270 = mat3(0, 0, 1, 0, 1, 0, -1, 0, 0);

void main()
{
    vec3 vertexPosition;
    if (TileRotation == 0.0) { vertexPosition = VertexPosition;}
    else if (TileRotation == 1.0) {vertexPosition = rotate90 * VertexPosition;}
    else if (TileRotation == 2.0) {vertexPosition = rotate180 * VertexPosition;}
    else vertexPosition = rotate270 * VertexPosition;

    vertexPosition = vec3(vertexPosition.x + VertexOffset.y, vertexPosition.y, vertexPosition.z + VertexOffset.x);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(vertexPosition, 1.0);
    uv = vec2(0);
    if (VertexPosition.x > 0) uv.x = 1.0;
    if (VertexPosition.z > 0) uv.y = 1.0;

    vec3 vertexPosition_cs = mat3(ViewMatrix) * vertexPosition;
    eyeDirection_cs = vec3(0,0,0) - vertexPosition_cs;

    vec3 lightPosition_cs = ( ViewMatrix * vec4(LightMatrix[0],1)).xyz;
	lightDirection_cs = lightPosition_cs + eyeDirection_cs;
}  