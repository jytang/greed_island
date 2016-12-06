#version 330 core
#define MAX_SHADOW_MAPS 10

struct Material {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 color;
    float ambient_coeff;
};

in vec3 frag_pos;
in vec3 frag_normal;
in vec4 frag_pos_light;

out vec4 color;

uniform sampler2D shadow_maps[MAX_SHADOW_MAPS];
uniform DirLight lights[MAX_SHADOW_MAPS];
uniform int num_lights_f;
uniform bool shadows_enabled;

uniform vec3 eye_pos;
uniform Material material;

vec3 colorify(vec3 normal, vec3 view_dir, vec3 light_dir, vec3 light_intensity, float ambient_coeff);
float calc_shadows(vec4 pos_from_light, vec3 light_dir);

void main()
{
    vec3 normal = normalize(frag_normal);
    vec3 view_dir = normalize(eye_pos - frag_pos);

    vec3 result = colorify(normal, view_dir);
    color = vec4(result, 1.0f);
}

float calc_shadows(vec4 pos_from_light, vec3 light_dir)
{
	vec3 clip_coords = pos_from_light.xyz / pos_from_light.w;
	// Transform to range of [0, 1] to fit depth map
	clip_coords = clip_coords * 0.5 + 0.5;
	float current_depth = clip_coords.z;

	if (current_depth > 1.0) {
		return 0.0;
	}

	float bias = max(0.05 * (1.0 - dot(normalize(frag_normal), light_dir)), 0.005);  
	float shadow = 0.0;

	vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadow_map, clip_coords.xy + vec2(x, y) * texelSize).r; 
			shadow += current_depth - bias > pcfDepth ? 1.0 : 0.0;       
		}    
	}
	shadow /= 9.0;

	return shadow;
}

vec3 colorify(vec3 normal, vec3 view_dir)
{
	vec3 light_dir = normalize(-dir_light.direction);
    vec3 light_intensity = dir_light.color;

    // Diffuse: c_d = c_l * k_d * dot(n, L)
    vec3 diffuse = light_intensity * material.diffuse *
        max(dot(normal, light_dir), 0.0);

    // Specular: c_s = c_l * k_s * dot(n, h)^s
    vec3 specular = light_intensity * material.specular *
        pow(max(dot(normal, normalize(light_dir + view_dir)), 0.0), material.shininess);

    // Ambient: c_a (ambient color) * k_a (coeff)
    vec3 ambient = material.ambient * dir_light.ambient_coeff;

	float shadow = 0;
	if (shadows_enabled)
		shadow = calc_shadows(frag_pos_light, light_dir);
    return (1.0 - shadow) * (diffuse + specular) + ambient;
}