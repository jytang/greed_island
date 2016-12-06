#pragma once
#include "shader.h"

#include <glm/glm.hpp>
#include <vector>

class ShadowShader :
	public Shader
{
public:
	std::vector<GLuint> FBOs;
	std::vector<GLuint> depth_textures;
	std::vector<glm::mat4> light_matrices;
	std::vector<glm::vec3> light_positions;
	std::vector<glm::mat4> light_projs;
	std::vector<unsigned int> sizes;

	unsigned int curr;

	ShadowShader(GLuint shader_id);
	void set_material(Material m);
	void draw(Geometry *g, glm::mat4 to_world);
};

