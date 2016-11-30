#pragma once

#include <GL/glew.h>
#include "geometry.h"

#include <vector>
#include <time.h>

#include "util.h"

class GeometryGenerator
{
public:
	static std::vector<Geometry *> geometries;

	static void clean_up();

	static Geometry *generate_cube(GLfloat scale, bool with_normals);
	static Geometry *generate_sphere(GLfloat radius, GLuint divisions);
	static Geometry *generate_cylinder(GLfloat radius, GLfloat height, GLuint divisions);
	static Geometry *generate_plane(GLfloat scale);
	static Geometry *generate_bezier_plane(GLfloat radius, GLuint num_curves, GLuint segmentation, GLfloat waviness, unsigned int seed);
};