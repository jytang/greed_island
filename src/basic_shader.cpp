#include "basic_shader.h"
#include "shader_manager.h"
#include "shadow_shader.h"

#include <iostream>

const unsigned int NUM_LIGHTS = 2;

BasicShader::BasicShader(GLuint shader_id) : Shader(shader_id) {}

void BasicShader::set_material(Material m)
{
	glUniform3f(glGetUniformLocation(shader_id, "material.diffuse"), m.diffuse.x, m.diffuse.y, m.diffuse.z);
	glUniform3f(glGetUniformLocation(shader_id, "material.specular"), m.specular.x, m.specular.y, m.specular.z);
	glUniform3f(glGetUniformLocation(shader_id, "material.ambient"), m.ambient.x, m.ambient.y, m.ambient.z);
	glUniform1f(glGetUniformLocation(shader_id, "material.shininess"), m.shininess);
	glUniform1i(glGetUniformLocation(shader_id, "shadows_enabled"), m.shadows);
}

void BasicShader::draw(Geometry *g, glm::mat4 to_world)
{
	glUniform1i(glGetUniformLocation(shader_id, "num_lights_v"), NUM_LIGHTS);
	glUniform1i(glGetUniformLocation(shader_id, "num_lights_f"), NUM_LIGHTS);

	// Bind depth texture from shadow shader, if it exists.
	ShadowShader * ss = (ShadowShader *) ShaderManager::get_shader_program("shadow");
	if (ss)
	{
		for (int i = 0; i < NUM_LIGHTS; ++i) {
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "light_matrix"), 1, GL_FALSE, &ss->light_matrices[i][0][0]);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, ss->depth_textures[i]);

			// Basic lighting
			glm::vec3 light_pos = ss->light_positions[i];
			glUniform3f(glGetUniformLocation(shader_id, "dir_light.direction"), -light_pos.x, -light_pos.y, -light_pos.z);
			glUniform3f(glGetUniformLocation(shader_id, "dir_light.color"), 1.f, 1.f, 1.f);
			glUniform1f(glGetUniformLocation(shader_id, "dir_light.ambient_coeff"), 0.2f);
		}
	}

	// Send camera position for shading
	glUniform3f(glGetUniformLocation(shader_id, "eye_pos"), cam_pos.x, cam_pos.y, cam_pos.z);
	// Send projection and view matrices
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "projection"), 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "view"), 1, GL_FALSE, &V[0][0]);
	// Model matrix
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "model"), 1, GL_FALSE, &to_world[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(shader_id, "mesh_model"), 1, GL_FALSE, &mesh_model[0][0]);
	// Bind geometry and draw
	g->bind();
	g->draw();
	glBindVertexArray(0);
}
