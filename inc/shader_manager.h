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

#include "shader.h"

class ShaderManager
{
private:
	std::map<const char*, Shader*> shaders;
	Shader *default_shader;
public:
	ShaderManager();
	~ShaderManager();
	void create_shader_program(const char *type);
	Shader* get_shader_program(const char *type);
	void set_default(const char *type);
	Shader* get_default();
};

