#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 mv_matrix;
uniform mat4 mvp_matrix;
uniform mat4 normal_matrix;
uniform vec4 Kd;

struct Light
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};


uniform Light light;


out vec2 tex;
out vec3 varying_light_direction;
out vec3 varying_vertPos;
out vec3 varying_normal;
out vec3 varying_HalfVector;

void main()
{
	gl_Position = mvp_matrix * vec4(position, 1.0);
	tex = texCoord;
	
	varying_normal = (normal_matrix * vec4(normal, 1.0f)).xyz;

	varying_vertPos = (mv_matrix * vec4(position, 1.0f)).xyz;

	//vec3 varying_lightPos = (mv_matrix * vec4(light.position, 1.0f)).xyz;

	//varying_light_direction = normalize(light.position - varying_vertPos);

	//vec3 varying_lightPos = (mv_matrix * vec4(light.position, 1.0f)).xyz;

	varying_light_direction = normalize(light.position - varying_vertPos);

	varying_HalfVector = normalize((varying_light_direction - varying_vertPos).xyz);

}