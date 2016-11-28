#pragma once

#include <GL/glew.h>
#include "geometry.h"

#include <vector>

class GeometryGenerator
{
public:
	static std::vector<Geometry *> geometries;

	static void clean_up();

	static Geometry *generate_cube(GLfloat scale, bool with_normals);
	static Geometry *generate_sphere(GLfloat radius, GLuint divisions);
	static Geometry *generate_cylinder(GLfloat radius, GLfloat height, GLuint divisions);
	static Geometry *generate_plane(GLfloat scale);
};

