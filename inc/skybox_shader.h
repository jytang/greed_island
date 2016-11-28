#pragma once

#include "shader.h"

class SkyboxShader :
	public Shader
{
public:
	GLuint texture_id;

	SkyboxShader(GLuint shader_id);
	void load_cubemap();
	void set_material(Material m);
	void draw(Geometry *g, glm::mat4 to_world);
};

