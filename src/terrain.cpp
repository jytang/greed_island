#include "terrain.h"

std::vector<std::vector<GLfloat> > Terrain::height_map;

void Terrain::generate_height_map(GLuint size, GLfloat max_height, GLint village_diameter, GLfloat scale, GLuint seed = 0)
{	
	unsigned int middle = ((size - 1) / 2); //Size is always odd
	
	//All points initialized at -1
	for (unsigned int i = 0; i < size; i++)
	{
		height_map.push_back(std::vector<GLfloat>());
		for (unsigned int j = 0; j < size; j++)
		{
			height_map[i].push_back(-1.f);
		}
	}	

	//fprintf(stderr, "Height Map Size: %d\t%d\t%d\n", size, height_map.size(), height_map[1].size());
	//fprintf(stderr, "Test Value: %f\n", height_map[0][0]);
	//fprintf(stderr, "Height Map Size: %d\t%f\n", test.size(), test[0]);

	//Initialize Four corners to be ground level
	height_map[0][0] = 0;
	height_map[0][size - 1] = 0;
	height_map[size - 1][0] = 0;
	height_map[size - 1][size - 1] = 0;

	//Initialize all sides ot be ground level
	for (unsigned int i = 0; i < size; i++)
	{
		height_map[0][i] = 0;
		height_map[size - 1][i] = 0;
		height_map[i][0] = 0;
		height_map[i][size - 1] = 0;
	}

	//Set Plateau! MaxHeight defined, and will be a fixed circle using VillageDiamteter
	//TODO: Later, for now center will just be the highest point;
	height_map[middle][middle] = max_height;
	//For now, square plateau
	unsigned int village_mid = village_diameter / 2;
	for (unsigned int i = middle - village_mid; i <= middle + village_mid; i++)
	{
		for (unsigned int j = middle - village_mid; j <= middle + village_mid; j++)
		{
			height_map[i][j] = max_height;
		}
	}


	//Diamond Square Algorithm
	if (seed != 0)
		srand(seed);

	unsigned int stepsize = size - 1;

	//Recursively creates height map
	diamond_square(stepsize, size, scale);
}

void Terrain::diamond_square(unsigned int step, unsigned int size, float scale)
{
	if (step <= 1)
		return;

	unsigned int halfstep = step / 2;

	for (unsigned int x = halfstep; x < size; x += step)
	{
		for (unsigned int y = halfstep; y < size; y += step)
		{
			diamond_step(x, y, step, size, (((float)(rand() % 101) / 100.f) * 2.f - 1) *scale);
		}
	}

	for (unsigned int x = 0; x < size; x += step)
	{
		for (unsigned int y = 0; y < size; y += step)
		{
			if (x + halfstep < size)
				square_step(x + halfstep, y, step, size, (((float)(rand() % 101) / 100.f) * 2.f - 1) *scale);
			if (y + halfstep < size)
				square_step(x, y + halfstep, step, size, (((float)(rand() % 101) / 100.f) * 2.f - 1) *scale);
		}
	}

	// (((float)(rand() % 101) / 100.f) * 2 - 1) *

	float roughness = 0.5f; //higher is smoother, lower is rougher

	scale *= (float) glm::pow(2.f, -roughness);
	
	diamond_square(step / 2, size, scale);
}

void Terrain::diamond_step(unsigned int x, unsigned int y, unsigned int step, unsigned int size, float scale)
{
	if (height_map[x][y] != -1)
		return;

	//fprintf(stderr, "Diamond on point %u, %u\n", x, y, size);

	unsigned int halfstep = step / 2;

	// a     b 
	//
	//    x
	//
	// c     d

	float a, b, c, d;
	float num = 4.0f;

	if (x >= halfstep && y >= halfstep)
		a = height_map[x - halfstep][y - halfstep];
	else
		a = 0;
	if (x + halfstep < size && y >= halfstep)
		b = height_map[x + halfstep][y - halfstep];
	else
		b = 0;
	if (x >= halfstep && y + halfstep < size)
		c = height_map[x - halfstep][y + halfstep];
	else
		c = 0;
	if (x + halfstep < size && y + halfstep < size)
		d = height_map[x + halfstep][y + halfstep];
	else
		d = 0;

	float r = (float)(rand() % 101) / 100.f;
	//fprintf(stderr, "Random: %f\n", r);

	//fprintf(stderr, "Using: %.2f, %.2f, %.2f, %.2f and %.2f\n", a, b, c, d, num);

	height_map[x][y] = ((a + b + c + d) / num) +(r * scale);
	if (height_map[x][y] < 0)
		height_map[x][y] = 0;
}

void Terrain::square_step(unsigned int x, unsigned int y, unsigned int step, unsigned int size, float scale)
{
	if (height_map[x][y] != -1)
		return;

	//fprintf(stderr, "Square on point %u, %u\n", x, y);

	unsigned int halfstep = step / 2;

	//   c
	//
	//a  x  b
	//
	//   d

	float a, b, c, d;	
	float num = 4.0f;

	if (x >= halfstep)
		a = height_map[x - halfstep][y];
	else
		a = 0;
	if (x + halfstep < size)
		b = height_map[x + halfstep][y];
	else
		b = 0;	
	if (y >= halfstep)
		c = height_map[x][y - halfstep];
	else
		c = 0;	
	if (y + halfstep < size)
		d = height_map[x][y + halfstep];
	else
		d = 0;

	if (x == 0 || y == 0 || x == size - 1 || y == size - 1)
		num = 3.0f;
	

	//fprintf(stderr, "Using: %.2f, %.2f, %.2f, %.2f and %.2f\n", a, b, c, d, num);

	float r = ((float)(rand() % 101) / 100.f);

	height_map[x][y] = ((a + b + c + d) / num) +(r * scale);
	if (height_map[x][y] < 0)
		height_map[x][y] = 0;
}

float Terrain::height_lookup(float x, float y, float length)
{
	//Assumes that terrain will be centered at origin and that terrain is square

	//fprintf(stderr, "Orig: %f, %f\n", x, y);

	//Translate to height_map coordinates
	float mid = length / 2.f;
	x = ((x + mid) / length) * ((float) height_map.size() - 1.f);
	y = ((y + mid) / length) * ((float) height_map.size() - 1.f);

	int x0 = (int) floorf(x);
	int x1 = (int) ceilf(x);
	int y0 = (int) floorf(y);
	int y1 = (int) ceilf(y);

	float rx, ry; //Ratios

	//Bilinear Interpolation
	if (x1 != x0)
		rx = (x - x0) / (x1 - x0);
	else
		rx = 0.0f;
	if (y1 != y0)
		ry = (y - y0) / (y1 - y0);
	else
		ry = 0.0f;

	//fprintf(stderr, "Test: %d, %d, %d, %d, %f, %f\n", x0, x1, y0, y1, x, y);

	float h0 = (height_map[x0][y0] * (1.f - rx)) + (height_map[x1][y0] * rx);
	float h1 = (height_map[x0][y1] * (1.f - rx)) + (height_map[x1][y1] * rx);

	return (h0 * (1.f - ry)) + (h1 * ry);
}