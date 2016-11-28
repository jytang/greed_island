#pragma once
#include <GL/glew.h>
#include "geometry.h"
#include "material.h"
#include "shader.h"

struct Mesh
{
	Geometry *geometry;
	Material material;
	Shader *shader;
};
