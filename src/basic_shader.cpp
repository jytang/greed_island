#include "basic_shader.h"
#include "shader_manager.h"
#include "shadow_shader.h"

#include <iostream>

BasicShader::BasicShader(GLuint shader_id) : Shader(shader_id) {}

void BasicShader::set_material(Material m)
{
	glUniform3f(glGetUniformLocation(shader_id, "material.diffuse"), m.diffuse.x, m.diffuse.y, m.diffuse.z);
	glUniform3f(glGetUniformLocation(shader_id, "material.specular"), m.specular.x, m.specular.y, m.specular.z);
	glUniform3f(glGetUniformLocation(shader_id, "material.ambient"), m.ambient.x, m.ambient.y, m.ambient.z);
	glUniform1f(glGetUniformLocation(shader_id, "material.shininess"), m.shininess);

	// Basic lighting
	glUniform3f(glGetUniformLocation(shader_id, "dir_light.direction"), 0.f, -2.f, 1.f);
	glUniform3f(glGetUniformLocation(shader_id, "dir_light.color"), 1.f, 1.f, 1.f);
	glUniform1f(glGetUniformLocation(shader_id, "dir_light.ambient_coeff"), 0.2f);
}

void BasicShader::draw(Geometry *g, glm::mat4 to_world)
{
	// Bind depth texture from shadow shader, if it exists.
	ShadowShader * ss = (ShadowShader *) ShaderManager::get_shader_program("shadow");
	if (ss)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader_id, "light_matrix"), 1, GL_FALSE, &ss->light_matrix[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ss->shadow_map_tex);
	}

	// Send camera position for shading
	glUniform3f(glGetUniformLocation(shader_id, "eye_pos"), cam_pos.x, cam_pos.y, cam_pos.z);
	// Send projection and view matrices
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "projection"), 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "view"), 1, GL_FALSE, &V[0][0]);
	// Model matrix
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "model"), 1, GL_FALSE, &to_world[0][0]);
	// Bind geometry and draw
	g->bind();
	g->draw();
	glBindVertexArray(0);
}
