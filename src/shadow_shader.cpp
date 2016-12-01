#include "shadow_shader.h"

#include <glm/gtc/matrix_transform.hpp>

ShadowShader::ShadowShader(GLuint shader_id) : Shader(shader_id)
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Generate shadow map texture.
	size = 1024;
	glGenTextures(1, &shadow_map_tex);
	glBindTexture(GL_TEXTURE_2D, shadow_map_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map_tex, 0);
	// Don't draw to colour buffer.
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::vec3 light_pos = glm::vec3(0.f, 2, 1);
	glm::mat4 light_proj = glm::ortho(-200.f, 200.f, -200.f, 200.f, -100.f, 100.f);
	//glm::mat4 light_proj = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 10.f);
	glm::mat4 light_view = glm::lookAt(light_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_matrix = light_proj * light_view;
}

void ShadowShader::set_material(Material m)
{
}

void ShadowShader::draw(Geometry *g, glm::mat4 to_world)
{
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "model"), 1, GL_FALSE, &to_world[0][0]);
	g->bind();
	g->draw();
	glBindVertexArray(0);
}