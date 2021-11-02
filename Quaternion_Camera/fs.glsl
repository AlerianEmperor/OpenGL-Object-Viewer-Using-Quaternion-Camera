#version 430

out vec4 frag_color;
uniform vec4 Kd;
uniform sampler2D DiffuseTexture;
uniform sampler2D MaskTexture;
uniform int useKd;
uniform int HasMask;

in vec2 tex;
in vec3 varying_light_direction;
in vec3 varying_normal;
in vec3 varying_vertPos;
in vec3 varying_HalfVector;

struct Light
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};

uniform vec4 globalAmbient;

uniform Light light;

void main()
{
	vec4 color;
	if(useKd == 0)
	{
		//vec4 a = texture2D(DiffuseTexture, tex);

		//if(a.w < 0.5f)
		//	discard;
		//else
		//	color = a;

		color = texture2D(DiffuseTexture, tex);
	}
	else
		color = Kd;

	//vec3 c_light_direction = vec3(0.1,0.1,0.1);

	vec3 L = normalize(varying_light_direction);
	vec3 N = normalize(varying_normal);
	vec3 H = normalize(varying_HalfVector);

	frag_color = color * (light.ambient + light.diffuse * max(dot(N, L), 0.0f) + light.specular * pow(max(dot(H, N) * 0.9f, 0.0f), 30.0f)); //pow(max(dot(H, N), 0.0f), 2000.0f);


	//frag_color = color * (0.2f + 0.4f * max(dot(L, N), 0.0f))
	//+ 0.6f * pow(max(dot(H, N) * 0.9f, 0.0f), 30.0f); //pow(max(dot(H, N), 0.0f), 2000.0f);

}