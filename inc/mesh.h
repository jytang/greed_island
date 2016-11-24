#pragma once
#include <GL/glew.h>
#include "geometry.h"
#include "material.h"

struct Mesh
{
	Geometry *geometry;
	Material material;
	GLuint shader;
};