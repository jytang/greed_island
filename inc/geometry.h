#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Geometry
{
public:
	bool has_texture = false;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> tex_coords;
	std::vector<GLuint> indices;
	GLenum draw_type = GL_TRIANGLES;

	Geometry();
	~Geometry();

	void populate_buffers();
private:
	GLuint VAO, VBO, NBO, TBO, EBO;
};