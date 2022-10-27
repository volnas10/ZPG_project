#version 330 core

in vec2 UV;
in vec3 vertex_position_worldspace;
in vec3 normal_cameraspace;
in vec3 eye_direction_cameraspace;
in vec3 light_direction_cameraspace;

out vec4 color;

uniform sampler2D texture_sampler;
uniform mat3 light_matrix;

layout(std140) uniform Material {
	vec4 diffuse_color; // r, g, b | texture_index
	vec4 specular_color;
	vec4 ambient_color;
	vec4 emissive_color;
	vec4 transparent_color; // r, g, b | opacity
	vec4 reflective_color; // r, g, b | reflectivity
	float refraction_index;
	float shininess;
	float shininess_strength;
} material;

void main(){

	//Material material = materials[material_index];

	vec3 LightColor = vec3(light_matrix[1].x, light_matrix[1].y, light_matrix[1].z);
	float LightPower = light_matrix[2].x;
	vec3 light_position = light_matrix[0];
	
	vec3 MaterialDiffuseColor = vec3(0.8, 0.8, 0.8);
	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.1,0.1,0.1);

	// If material has texture, use it
	vec3 diffuse_color;
	vec3 ambient_color;
	if (material.diffuse_color.w >= 0) {
		diffuse_color = texture(texture_sampler, UV).rgb;
		ambient_color = vec3(0.1,0.1,0.1) * diffuse_color;
	}
	else {
		diffuse_color = material.diffuse_color.xyz;
		ambient_color = material.ambient_color.xyz;
	}

	// Distance to the light
	float distance = length( light_position - vertex_position_worldspace );

	vec3 n = normalize( normal_cameraspace );
	vec3 l = normalize( light_direction_cameraspace );
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	vec3 E = normalize(eye_direction_cameraspace);
	vec3 R = reflect(-l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	//color += ambient_color;
	//color += diffuse_color * LightColor * LightPower * cosTheta / (distance * distance);
	//color += mix(material.specular_color.xyz, LightColor, 0.5) * LightPower *
	//			pow(cosAlpha, material.shininess) / (distance*distance) * material.shininess_strength;
	color.rgb = 
		// Ambient : simulates indirect lighting
		ambient_color +
		// Diffuse : "color" of the object
		diffuse_color * LightColor * LightPower * cosTheta / distance +
		// Specular : reflective highlight, like a mirror
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / distance;

	color.a = material.transparent_color.a;
}