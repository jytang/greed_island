#pragma once

#define GLFW_INCLUDE_GLEXT
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

class ShaderManager
{
private:
	std::map<const char*, GLuint> shaders;
	GLuint default_shader;
public:
	ShaderManager();
	~ShaderManager();
	void create_shader_program(const char *type);
	GLuint get_shader_program(const char *type);
	void set_default(const char *type);
	GLuint get_default();
};

