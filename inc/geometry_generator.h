#pragma once

#include <GL/glew.h>
#include "geometry.h"
#include "terrain.h"

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
	static Geometry *generate_cylinder(GLfloat radius, GLfloat height, GLuint divisions, bool is_centered);
	static Geometry *generate_plane(GLfloat scale);
	static Geometry *generate_bezier_plane(GLfloat radius, GLuint num_curves, GLuint segmentation, GLfloat waviness, unsigned int seed);
	//static Geometry *generate_grid(GLint size_modifier, GLfloat max_height, GLint village_diameter, GLfloat scale, GLuint seed);
	static Geometry *generate_terrain(GLfloat size, GLint num_points_side, GLfloat min_height, GLfloat max_height, bool normals_up, std::vector<std::vector<GLfloat> > &height_map);
};