#include "terrain.h"
#include "util.h"

float smoothness = 1.2f; //Previously called roughness. higher is smoother, lower is rougher
bool allow_below_ground = false;

std::vector<std::vector<GLfloat> > Terrain::generate_height_map(GLuint size, GLfloat max_height, GLint village_diameter, GLfloat scale, bool ramp, bool allow_dips, float smooth_value, GLuint seed = 0)
{
	std::vector<std::vector<GLfloat> > height_map;
	unsigned int middle = ((size - 1) / 2); //Size is always odd

	smoothness = smooth_value;
	allow_below_ground = allow_dips;
	
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

	//Offset of Village on Island, Round Plateau.
	float r_ratio_offset = 0.5; //Row <----Currnently, Code only works for when village is at center of island.
	float c_ratio_offset = 0.5; //Col
	unsigned int village_mid_r = (unsigned int) (size * r_ratio_offset);
	unsigned int village_mid_c = (unsigned int) (size * c_ratio_offset);

	unsigned int radius = village_diameter / 2;

	for (unsigned int r = village_mid_r - radius; r <= village_mid_r + radius; r++) 
	{
		for (unsigned int c = village_mid_c - radius; c <= village_mid_c + radius; c++) 
		{
			float dist = glm::distance(glm::vec2(r, c), glm::vec2(village_mid_r, village_mid_c));
			//fprintf(stderr, "Dist: %f\n", dist);
			if (dist <= radius)
			{
				height_map[r][c] = max_height;
			}
		}
	}
	
	//Square Plateau
	//height_map[middle][middle] = max_height;
	////For now, square plateau
	//unsigned int village_mid = village_diameter / 2;
	//for (unsigned int i = middle - village_mid; i <= middle + village_mid; i++)
	//{
	//	for (unsigned int j = middle - village_mid; j <= middle + village_mid; j++)
	//	{
	//		height_map[i][j] = max_height;
	//	}
	//}


	//Diamond Square Algorithm
	if (seed != 0)
		srand(seed);

	unsigned int stepsize = size - 1;

	//Recursively creates height map
	diamond_square(stepsize, size, scale, height_map);

	//Depresses the Plateau for a cooler effect!
	max_height *= 0.8f; //Amount of depression.

	for (unsigned int r = village_mid_r - radius; r < village_mid_r + radius; r++)
	{
		for (unsigned int c = village_mid_c - radius; c < village_mid_c + radius; c++)
		{
			float dist = glm::distance(glm::vec2(r, c), glm::vec2(village_mid_r, village_mid_c));
			//fprintf(stderr, "Dist: %f\n", dist);
			if (dist <= radius)
			{
				height_map[r][c] = max_height;
			}
		}
	}	

	//Makes a ramp to the plateau	
	if (ramp)
	{
		float start_slope = 1.0f;
		float end_slope = 20.0f;
		float widen_offset = 5.0f;
		unsigned int initial_widen = 5;

		for (unsigned int c = village_mid_c + radius; c < village_mid_c + radius + end_slope; c++)
		{
			float ratio = (village_mid_c + radius + end_slope - (float)c) / (end_slope - start_slope);
			unsigned int widen = (unsigned int)(widen_offset * (1.0f - ratio));
			unsigned int opening_size = initial_widen + widen;
			for (unsigned int r = village_mid_r - opening_size; r < village_mid_r + opening_size; r++)
			{
				if (c <= village_mid_c + radius + start_slope)
				{
					height_map[r][c] = max_height;
				}
				else
				{
					float diff = max_height - height_map[r][c];
					height_map[r][c] = max_height - (diff * (1.f - ratio));
				}
			}
		}
	}

	return height_map;
}

void Terrain::diamond_square(unsigned int step, unsigned int size, float scale, std::vector<std::vector<GLfloat> > &height_map)
{
	if (step <= 1)
		return;

	unsigned int halfstep = step / 2;

	for (unsigned int x = halfstep; x < size; x += step)
	{
		for (unsigned int y = halfstep; y < size; y += step)
		{
			diamond_step(x, y, step, size, Util::random(-1.f, 1.f) * scale, height_map);
		}
	}

	for (unsigned int x = 0; x < size; x += step)
	{
		for (unsigned int y = 0; y < size; y += step)
		{
			if (x + halfstep < size)
				square_step(x + halfstep, y, step, size, Util::random(-1.f, 1.f)  *scale, height_map);
			if (y + halfstep < size)
				square_step(x, y + halfstep, step, size, Util::random(-1.f, 1.f)  *scale, height_map);
		}
	}

	scale *= (float)glm::pow(2.f, -smoothness);

	diamond_square(step / 2, size, scale, height_map);
}

void Terrain::diamond_step(unsigned int x, unsigned int y, unsigned int step, unsigned int size, float scale, std::vector<std::vector<GLfloat> > &height_map)
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

	//float r = (float)(rand() % 101) / 100.f;
	float r = Util::random(0.f, 1.f);
	//fprintf(stderr, "Random: %f\n", r);

	//fprintf(stderr, "Using: %.2f, %.2f, %.2f, %.2f and %.2f\n", a, b, c, d, num);

	float sum = (a + b + c + d);

	height_map[x][y] = (sum / num) +(r * scale);
	if (!allow_below_ground && height_map[x][y] < 0)
		height_map[x][y] = 0;
}

void Terrain::square_step(unsigned int x, unsigned int y, unsigned int step, unsigned int size, float scale, std::vector<std::vector<GLfloat> > &height_map)
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
		
	float r = Util::random(0.f, 1.f);

	float sum = (a + b + c + d);

	height_map[x][y] = (sum / num) +(r * scale);
	if (!allow_below_ground && height_map[x][y] < 0)
		height_map[x][y] = 0;
}

float Terrain::height_lookup(float x, float y, float length, std::vector<std::vector<GLfloat> > &height_map)
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
