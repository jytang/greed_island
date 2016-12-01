#version 330 core
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

uniform sampler2D shadow_map;
uniform vec3 eye_pos;
uniform Material material;
uniform DirLight dir_light;

vec3 colorify(vec3 normal, vec3 view_dir, vec3 light_dir, vec3 light_intensity, float ambient_coeff);
float calc_shadows(vec4 pos_from_light, vec3 light_dir);

void main()
{
    vec3 normal = normalize(frag_normal);
    vec3 view_dir = normalize(eye_pos - frag_pos);
	vec3 light_dir = normalize(-dir_light.direction);
    vec3 light_intensity = dir_light.color;
    vec3 result = colorify(normal, view_dir, light_dir, light_intensity, dir_light.ambient_coeff);

    color = vec4(result, 1.0f);
}

float calc_shadows(vec4 pos_from_light, vec3 light_dir)
{
	//return 0;
	vec3 clip_coords = pos_from_light.xyz / pos_from_light.w;
	// Transform to range of [0, 1] to fit depth map
	clip_coords = clip_coords * 0.5 + 0.5; 
	float closest_depth = texture(shadow_map, clip_coords.xy).r;
	float current_depth = clip_coords.z;

	float bias = max(0.05 * (1.0 - dot(normalize(frag_normal), light_dir)), 0.005);  
	float shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;

	return shadow;
}

vec3 colorify(vec3 normal, vec3 view_dir, vec3 light_dir, vec3 light_intensity, float ambient_coeff)
{
    // Diffuse: c_d = c_l * k_d * dot(n, L)
    vec3 diffuse = light_intensity * material.diffuse *
        max(dot(normal, light_dir), 0.0);

    // Specular: c_s = c_l * k_s * dot(n, h)^s
    vec3 specular = light_intensity * material.specular *
        pow(max(dot(normal, normalize(light_dir + view_dir)), 0.0), material.shininess);

    // Ambient: c_a (ambient color) * k_a (coeff)
    vec3 ambient = material.ambient * ambient_coeff;

	float shadow = calc_shadows(frag_pos_light, light_dir);
    return (1.0 - shadow) * (diffuse + specular) + ambient;
}
