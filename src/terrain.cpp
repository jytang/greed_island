#include "terrain.h"

std::vector<std::vector<GLfloat> > Terrain::height_map;

void Terrain::generate_height_map(GLuint size, GLfloat max_height, GLint village_diameter, GLuint seed = 0)
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

	//Set Plateau! MaxHeight defined, and will be a fixed circle using VillageDiamteter
	//TODO: Later, for now center will just be the highest point;
	height_map[middle][middle] = max_height;

	//Diamond Square Algorithm
	if (seed == 0)
		srand((unsigned int)time(NULL));
	else
		srand(seed);

	unsigned int stepsize = size - 1;
	float scale = 100.f;

	//Recursively creates height map
	diamond_square(stepsize, size, scale);
}

void Terrain::diamond_square(unsigned int step, unsigned int size, float scale)
{
	if (step < 1)
		return;

	unsigned int halfstep = step / 2;

	for (unsigned int y = halfstep; y < size; y += step)
	{
		for (unsigned int x = halfstep; x < size; x += step)
		{
			diamond_step(x, y, step, (rand() % 2 - 1) * scale);
		}
	}

	for (unsigned int y = 0; y < size; y += step)
	{
		for (unsigned int x = 0; x < size; x += step)
		{
			if (x + halfstep < size)
				square_step(x + halfstep, y, step, (rand() % 2 - 1) * scale);
			if (y + halfstep < size)
				square_step(x, y + halfstep, step, (rand() % 2 - 1) * scale);
		}
	}
	
	diamond_square(step / 2, size, scale / 2.f);
}

void Terrain::diamond_step(unsigned int x, unsigned int y, unsigned int size, float scale)
{

	//Check if point already set
	if (height_map[x][y] >= 0)
		return;

	unsigned int halfstep = size / 2;

	// a     b 
	//
	//    x
	//
	// c     d

	float a, b, c, d;

	if (x >= halfstep && y >= halfstep)
		a = height_map[x - halfstep][y - halfstep];
	else
		a = 0;
	if (x + halfstep < size && y >= halfstep)
		b = height_map[x + halfstep][y - halfstep];
	else
		b = 0;
	if (x >= halfstep && y + halfstep >= 0)
		c = height_map[x - halfstep][y + halfstep];
	else
		c = 0;
	if (x + halfstep >= 0 && y + halfstep >= 0)
		d = height_map[x + halfstep][y + halfstep];
	else
		d = 0;

	height_map[x][y] = (a + b + c + d) / 4.f + ((float) (rand() % 101) / 100.f) * scale;
	if (height_map[x][y] < 0)
		height_map[x][y] = 0;
}

void Terrain::square_step(unsigned int x, unsigned int y, unsigned int size, float scale)
{
	//Check if point already set
	if (height_map[x][y] >= 0)
		return;

	unsigned int halfstep = size / 2;

	//   c
	//
	//a  x  b
	//
	//   d

	float a;
	float b;
	float c;
	float d;

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

	height_map[x][y] = (a + b + c + d) / 4.f + ((float) (rand() % 101) / 100.f) * scale;
	if (height_map[x][y] < 0)
		height_map[x][y] = 0;
}

float Terrain::height_lookup(int x, int z)
{
	return height_map[x][z];
}