#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <time.h>
#include <math.h>

class Terrain
{
private:
	static void diamond_square(unsigned int step, unsigned int size, float scale, std::vector<std::vector<GLfloat> > &);
	static void diamond_step(unsigned int x, unsigned int y, unsigned int step, unsigned int size, float scale, std::vector<std::vector<GLfloat> > &);
	static void square_step(unsigned int x, unsigned int y, unsigned int step, unsigned int size, float scale, std::vector<std::vector<GLfloat> > &);

public:
	static std::vector<std::vector<GLfloat> > generate_height_map(GLuint size, GLfloat max_height, GLint village_diameter, GLfloat scale, bool ramp, bool allow_dips, float smooth_value, GLuint seed);
	static float height_lookup(float x, float y, float length, std::vector<std::vector<GLfloat> > &);
};

