#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 frag_pos;
out vec3 frag_normal;
out vec4 frag_pos_light;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_matrix;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    frag_pos = vec3(model * vec4(position, 1.0f));
    frag_normal = mat3(transpose(inverse(model))) * normal;
	frag_pos_light = light_matrix * vec4(frag_pos, 1.0);
}
