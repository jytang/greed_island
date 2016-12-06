#version 330 core
#define MAX_LIGHT_MATRIX 10
#define MAX_LIGHT_COORDS 10

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 frag_pos;
out vec3 frag_normal;
out vec4 frag_pos_light[MAX_LIGHT_COORDS];

uniform int num_lights_v;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_matrix[MAX_LIGHT_MATRIX];
uniform mat4 mesh_model;

void main()
{
    gl_Position = projection * view * model * mesh_model * vec4(position, 1.0f);
    frag_pos = vec3(model * mesh_model * vec4(position, 1.0f));
    frag_normal = mat3(transpose(inverse(model * mesh_model))) * normal;

	for (int i = 0; i < num_lights; ++i)
		frag_pos_light[i] = light_matrix[i] * vec4(frag_pos, 1.0);
}
