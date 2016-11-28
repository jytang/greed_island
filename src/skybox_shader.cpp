#include "skybox_shader.h"
#include "util.h"

#include <vector>

GLuint VAO, VBO;

const GLfloat skybox_vertices[] = {
	// Positions
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

SkyboxShader::SkyboxShader(GLuint shader_id) : Shader(shader_id)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindVertexArray(0);

	glGenTextures(1, &texture_id);
	load_cubemap();
}

void SkyboxShader::load_cubemap()
{
	std::vector<const char*> faces;
	faces.push_back("\\right.ppm");
	faces.push_back("\\left.ppm");
	faces.push_back("\\top.ppm");
	faces.push_back("\\bottom.ppm");
	faces.push_back("\\back.ppm");
	faces.push_back("\\front.ppm");

	int width, height;
	unsigned char *image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	for (GLuint i = 0; i < faces.size(); ++i) {
		std::string path = "assets\\skybox\\low-res-cloudy" + std::string(faces[i]);
		image = Util::loadPPM(path.c_str(), width, height);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		delete image;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void SkyboxShader::set_material(Material m)
{

}

void SkyboxShader::draw(Geometry *g, glm::mat4 to_world)
{
	glDepthMask(GL_FALSE);
	// Strip translation from view matrix.
	glm::mat4 view = glm::mat4(glm::mat3(V));

	// Send projection and view matrices
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "projection"), 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "view"), 1, GL_FALSE, &view[0][0]);
	// Bind geometry and draw
	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader_id, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
}