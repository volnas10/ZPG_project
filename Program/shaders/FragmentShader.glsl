#version 330 core

// Interpolated values from the vertex shaders
in vec3 vertex_position_worldspace;
in vec3 normal_cameraspace;
in vec3 eye_direction_cameraspace;
in vec3 light_direction_cameraspace;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform vec3 light_position;

void main(){

	// Light emission properties
	// You probably want to put them as uniforms
	vec3 LightColor = vec3(1.0,1.0,1.0);
	float LightPower = 40.0f;
	
	// Material properties
	vec3 MaterialDiffuseColor = vec3(0.9, 0.9, 0.9);
	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

	// Distance to the light
	float distance = length( light_position - vertex_position_worldspace );

	// Normal of the computed fragment, in camera space
	vec3 n = normalize( normal_cameraspace );
	vec3 l = normalize( light_direction_cameraspace );
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	vec3 E = normalize(eye_direction_cameraspace);
	vec3 R = reflect(-l,n);

	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	color.rgb =
		MaterialAmbientColor +
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
	color.a = 1.0;
}