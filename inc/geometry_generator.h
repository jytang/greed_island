#pragma once

#include <GL/glew.h>
#include "geometry.h"

class GeometryGenerator
{
public:
	GeometryGenerator();
	~GeometryGenerator();

	static Geometry *generate_cube(GLfloat scale);
	static Geometry *generate_sphere(GLfloat radius, GLuint divisions);
	static Geometry *generate_cylinder(GLfloat radius, GLfloat height, GLuint divisions);
	static Geometry *generate_plane(GLfloat scale);
};

