#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <time.h>
#include <math.h>

class Terrain
{
private:
	static std::vector< std::vector<GLfloat> > height_map;

	static void diamond_square(unsigned int step, unsigned int size, float scale);
	static void diamond_step(unsigned int x, unsigned int y, unsigned int step, unsigned int size, float scale);
	static void square_step(unsigned int x, unsigned int y, unsigned int step, unsigned int size, float scale);

public:
	static void generate_height_map(GLuint size, GLfloat max_height, GLint village_diameter, GLfloat scale, GLuint seed);
	static float height_lookup(float x, float y, float length);
};
