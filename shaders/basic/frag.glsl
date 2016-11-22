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

    bool on;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float quadratic;
    float ambient_coeff;

    bool on;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float angle;
    float taper;
    float ambient_coeff;
    float quadratic;

    bool on;
};

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 color;

uniform vec3 eye_pos;
uniform Material material;
uniform DirLight dir_light;
uniform PointLight point_light;
uniform SpotLight spot_light;
uniform bool normal_colors;

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 view_dir);
vec3 calc_point_light(PointLight light, vec3 normal, vec3 view_dir, vec3 frag_pos);
vec3 calc_spot_light(SpotLight light, vec3 normal, vec3 view_dir, vec3 frag_pos);
vec3 colorify(vec3 normal, vec3 view_dir, vec3 light_dir, vec3 light_intensity, float ambient_coeff);

void main()
{
    vec3 norm = normalize(frag_normal);
    vec3 view_dir = normalize(eye_pos - frag_pos);

    vec3 result = vec3(0.0f);

    if (normal_colors) {
        result = norm;
    } else {
        result = calc_dir_light(dir_light, norm, view_dir);
        result += calc_point_light(point_light, norm, view_dir, frag_pos);
        result += calc_spot_light(spot_light, norm, view_dir, frag_pos);
    }

    color = vec4(result, 1.0f);
}

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 view_dir)
{
    if (light.on) {
        vec3 light_dir = normalize(-light.direction);
        vec3 light_intensity = light.color;
        return colorify(normal, view_dir, light_dir, light_intensity, light.ambient_coeff);
    } else {
        return vec3(0.0f);
    }
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 view_dir, vec3 frag_pos)
{
    if (light.on) {
        vec3 light_dir = normalize(light.position - frag_pos);

        float distance = length(light.position - frag_pos);
        float attenuation = 1.0f / (light.quadratic * distance * distance);
        vec3 light_intensity = light.color * attenuation;

        return colorify(normal, view_dir, light_dir, light_intensity, light.ambient_coeff);
    } else {
        return vec3(0.0f);
    }
}

vec3 calc_spot_light(SpotLight light, vec3 normal, vec3 view_dir, vec3 frag_pos)
{
    if (light.on) {
        vec3 light_dir = normalize(light.position - frag_pos);
        vec3 spotlight_dir = normalize(light.direction);

        vec3 light_intensity = vec3(0.0f);
        float distance = length(light.position - frag_pos);
        float attenuation = 1.0f / (light.quadratic * distance * distance);

        // Check that point is within the beam
        if (dot(-light_dir, spotlight_dir) > cos(light.angle)) {
            light_intensity = light.color * pow(dot(-light_dir, spotlight_dir), light.taper);
            light_intensity *= attenuation;
        }

        return colorify(normal, view_dir, light_dir, light_intensity, light.ambient_coeff);
    } else {
        return vec3(0.0f);
    }
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

    return diffuse + specular + ambient;
}
