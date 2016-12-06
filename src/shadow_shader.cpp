#include "shadow_shader.h"

#include <glm/gtc/matrix_transform.hpp>

const unsigned int NUM_LIGHTS = 2;

ShadowShader::ShadowShader(GLuint shader_id) : Shader(shader_id)
{
	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		GLuint FBO;
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// Generate shadow map texture of size 4096 and lower for other lights.
		unsigned int size;
		if (i == 0)
			size = 4096;
		else
			size = 1024;

		GLuint depth_texture;
		glGenTextures(1, &depth_texture);
		glBindTexture(GL_TEXTURE_2D, depth_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
		// Don't draw to colour buffer.
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		FBOs.push_back(FBO);
		depth_textures.push_back(depth_texture);
		light_matrices.push_back(glm::mat4(1.f));
		light_positions.push_back(glm::vec3(0.f));
		light_projs.push_back(glm::mat4(1.f));
		sizes.push_back(size);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowShader::set_material(Material m)
{
}

void ShadowShader::draw(Geometry *g, glm::mat4 to_world)
{
	// Recalculate light matrix based on current light position and light projection matrix
	glm::mat4 light_view = glm::lookAt(light_positions[curr], glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_matrices[curr] = light_projs[curr] * light_view;

	glUniformMatrix4fv(glGetUniformLocation(shader_id, "view_projection"), 1, GL_FALSE, &light_matrices[curr][0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "model"), 1, GL_FALSE, &to_world[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "mesh_model"), 1, GL_FALSE, &mesh_model[0][0]);
	g->bind();
	g->draw();
	glBindVertexArray(0);
}