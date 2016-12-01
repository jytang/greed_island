#include "geometry_generator.h"

std::vector<Geometry *> GeometryGenerator::geometries;

void GeometryGenerator::clean_up()
{
	for (auto it = geometries.begin(); it != geometries.end(); ++it)
		delete(*it);
}

Geometry * GeometryGenerator::generate_cube(GLfloat scale, bool has_normals)
{
	Geometry *cube = new Geometry();
	
	glm::vec3 v0 = { scale, scale, scale };
	glm::vec3 v1 = { -scale, scale, scale };
	glm::vec3 v2 = { scale, scale, -scale };
	glm::vec3 v3 = { -scale, scale, -scale };
	glm::vec3 v4 = { scale, -scale, scale };
	glm::vec3 v5 = { -scale, -scale, scale };
	glm::vec3 v6 = { scale, -scale, -scale };
	glm::vec3 v7 = { -scale, -scale, -scale };

	cube->vertices.push_back(v0);
	cube->vertices.push_back(v2);
	cube->vertices.push_back(v1);
	cube->vertices.push_back(v1);
	cube->vertices.push_back(v2);
	cube->vertices.push_back(v3);

	cube->vertices.push_back(v2);
	cube->vertices.push_back(v6);
	cube->vertices.push_back(v3);
	cube->vertices.push_back(v3);
	cube->vertices.push_back(v6);
	cube->vertices.push_back(v7);

	cube->vertices.push_back(v1);
	cube->vertices.push_back(v3);
	cube->vertices.push_back(v5);
	cube->vertices.push_back(v3);
	cube->vertices.push_back(v7);
	cube->vertices.push_back(v5);

	cube->vertices.push_back(v0);
	cube->vertices.push_back(v4);
	cube->vertices.push_back(v2);
	cube->vertices.push_back(v2);
	cube->vertices.push_back(v4);
	cube->vertices.push_back(v6);

	cube->vertices.push_back(v1);
	cube->vertices.push_back(v5);
	cube->vertices.push_back(v0);
	cube->vertices.push_back(v0);
	cube->vertices.push_back(v5);
	cube->vertices.push_back(v4);

	cube->vertices.push_back(v5);
	cube->vertices.push_back(v7);
	cube->vertices.push_back(v4);
	cube->vertices.push_back(v4);
	cube->vertices.push_back(v7);
	cube->vertices.push_back(v6);

	if (has_normals)
	{
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(0.f, 1.f, 0.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(0.f, 0.f, -1.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(-1.f, 0.f, 0.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(1.f, 0.f, 0.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(0.f, 0.f, 1.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(0.f, -1.f, 0.f));
	}

	for (int i = 0; i < cube->vertices.size(); ++i)
		cube->indices.push_back(i);

	cube->has_normals = has_normals;
	cube->populate_buffers();
	geometries.push_back(cube);
	return cube;
}

Geometry * GeometryGenerator::generate_sphere(GLfloat radius, GLuint divisions)
{
	return nullptr;
}

Geometry * GeometryGenerator::generate_cylinder(GLfloat radius, GLfloat height, GLuint divisions)
{
	return nullptr;
}

Geometry * GeometryGenerator::generate_plane(GLfloat scale)
{
	Geometry *plane = new Geometry();

	//Setting Y-Offset as 0, change by Rotating World
	glm::vec3 v0 = { scale, 0, scale };
	glm::vec3 v1 = { -scale, 0, scale };
	glm::vec3 v2 = { scale, 0, -scale };
	glm::vec3 v3 = { -scale, 0, -scale };

	glm::vec3 n = {0, 1.0f, 0};

	plane->vertices.push_back(v0);
	plane->vertices.push_back(v1);
	plane->vertices.push_back(v2);
	plane->vertices.push_back(v3);

	for (int i = 0; i < 4; i++)
		plane->normals.push_back(n);

	plane->indices.push_back(0);
	plane->indices.push_back(2);
	plane->indices.push_back(1);
	plane->indices.push_back(1);
	plane->indices.push_back(2);
	plane->indices.push_back(3);

	plane->populate_buffers();
	geometries.push_back(plane);

	return plane;
}

Geometry * GeometryGenerator::generate_terrain(GLint size, GLfloat min_height, GLfloat max_height)
{
	//Experimenting. Assumed that height map is already set up and size is same as height map size

	Geometry *terrain = new Geometry();

	//Create array of points using height map lookup function
	//Grid is same size as height map, and scales with height map size
	int middle = (size - 1) / 2;
	int min_x = -middle;
	int min_z = -middle;
	int max_x = middle;
	int max_z = middle;

	//Goes Square by Square
	for (int x = min_x; x < max_x; x++)
	{
		for (int z = min_z; z < max_z; z++)
		{
			float h1 = Terrain::height_lookup(x + middle, z + middle);
			float h2 = Terrain::height_lookup(x + 1 + middle, z + middle);
			float h3 = Terrain::height_lookup(x + middle, z + 1 + middle);
			float h4 = Terrain::height_lookup(x + 1 + middle, z + 1 + middle);

			if (h1 < min_height || h1 > max_height)
				continue;

			glm::vec3 v1 = glm::vec3(x, h1, z); //Upper Left
			glm::vec3 v2 = glm::vec3(x + 1, h2, z); //Upper Right
			glm::vec3 v3 = glm::vec3(x, h3, z + 1); //Lower Left
			glm::vec3 v4 = glm::vec3(x + 1, h4, z + 1);  //Lower Right)

	        //Make Two Triangles for Square
			terrain->vertices.push_back(v4);
			terrain->vertices.push_back(v2);
			terrain->vertices.push_back(v1);
			terrain->vertices.push_back(v1);
			terrain->vertices.push_back(v3);
			terrain->vertices.push_back(v4);

			//Get Normals for each vertex pushed (Right hand rule ftw)
			glm::vec3 n1 = glm::cross(v2 - v4, v1 - v4);
			glm::vec3 n2 = glm::cross(v1 - v2, v4 - v2);
			glm::vec3 n3 = glm::cross(v4 - v1, v2 - v1);
			glm::vec3 n4 = glm::cross(v3 - v1, v4 - v1);
			glm::vec3 n5 = glm::cross(v4 - v3, v1 - v3);
			glm::vec3 n6 = glm::cross(v1 - v4, v3 - v4);

			terrain->normals.push_back(n1);
			terrain->normals.push_back(n2);
			terrain->normals.push_back(n3);
			terrain->normals.push_back(n4);
			terrain->normals.push_back(n5);
			terrain->normals.push_back(n6);
		}
	}

	//Indices are just in order to make it easier
	for (int i = 0; i < terrain->vertices.size(); i++)
		terrain->indices.push_back(i);

	terrain->populate_buffers();
	geometries.push_back(terrain);

	return terrain;
}

/*
Geometry * GeometryGenerator::generate_grid(GLint size_modifier, GLfloat max_height, GLint village_diameter, GLfloat scale, GLuint seed = 0)
{		
	/*Note to self regarding terrain's smoothness: AustinPuk
		The reason why the current terrain has such low resolution is becuase after generating the height map,
		I just made a 1:1 relation of the height map's points to the vertices. This is actually pretty inefficient
		since the height map takes exponentially longer to calculate with more points, but we do not necessarily
		need more points on the height map itself to have a smoother, more realistic surface. Instead, I should
		really be using the height map as a proper lookup table and make the grid an aribtrary sized resolution of
		vertices, and then choosing the height of each pixel in the height map based on an averaged/nearest-neighor
		approach to get smoother surfaces.		
		This shouldn't take long to fix. In fact, I'll do it now....	
	star/

	//Create 2D Array of size (2^n + 1) for Height Map
	unsigned int size = (unsigned int)glm::pow(2, size_modifier) + 1;

	Geometry *terrain = new Geometry();

	Terrain::generate_height_map(size, max_height, village_diameter, scale, seed);

	//Create array of points using height map lookup function
	//Grid is same size as height map, and scales with height map size
	int middle = (size - 1) / 2;
	int min_x = -middle;
	int min_z = -middle;
	int max_x = middle;
	int max_z = middle;
			
	float land_threshold = 20.0f;
	
	//Goes Square by Square
	for (int x = min_x; x < max_x; x++)
	{
		for (int z = min_z; z < max_z; z++)
		{
			float h1 = Terrain::height_lookup(x + middle, z + middle);
			float h2 = Terrain::height_lookup(x + 1 + middle, z + middle);
			float h3 = Terrain::height_lookup(x + middle, z + 1 + middle);
			float h4 = Terrain::height_lookup(x + 1 + middle, z + 1 + middle);

			if (h1 < land_threshold)
				continue;

			glm::vec3 v1 = glm::vec3(x, h1, z); //Upper Left
			glm::vec3 v2 = glm::vec3(x + 1, h2, z); //Upper Right
			glm::vec3 v3 = glm::vec3(x, h3, z + 1); //Lower Left
			glm::vec3 v4 = glm::vec3(x + 1, h4, z + 1);  //Lower Right)

			//Make Two Triangles for Square
			terrain->vertices.push_back(v4);
			terrain->vertices.push_back(v2);
			terrain->vertices.push_back(v1);
			terrain->vertices.push_back(v1);
			terrain->vertices.push_back(v3);
			terrain->vertices.push_back(v4);

			//Get Normals for each vertex pushed (Right hand rule ftw)
			glm::vec3 n1 = glm::cross(v2 - v4, v1 - v4); 
			glm::vec3 n2 = glm::cross(v1 - v2, v4 - v2);
			glm::vec3 n3 = glm::cross(v4 - v1, v2 - v1);
			glm::vec3 n4 = glm::cross(v3 - v1, v4 - v1);
			glm::vec3 n5 = glm::cross(v4 - v3, v1 - v3);
			glm::vec3 n6 = glm::cross(v1 - v4, v3 - v4);		

			terrain->normals.push_back(n1);
			terrain->normals.push_back(n2);
			terrain->normals.push_back(n3);
			terrain->normals.push_back(n4);
			terrain->normals.push_back(n5);
			terrain->normals.push_back(n6);
		}
	}		

	//Indices are just in order to make it easier
	for (int i = 0; i < terrain->vertices.size(); i++)
		terrain->indices.push_back(i);	
	
	terrain->populate_buffers();
	geometries.push_back(terrain);		

	return terrain;
}
*/

Geometry * GeometryGenerator::generate_bezier_plane(GLfloat radius, GLuint num_curves, GLuint segmentation, GLfloat waviness, unsigned int seed = 0)
{
	Geometry *bez_plane = new Geometry();
	bez_plane->draw_type = GL_TRIANGLE_FAN;

	// Make bezier curves
	if (seed == 0)
		srand((unsigned int)time(NULL));
	else
		srand(seed);
	int num_points = num_curves * 3;
	std::vector<glm::vec3> control_points(num_points);
	for (int i = 0; i < num_points; ++i)
	{
		if (i % 3 == 0) continue; // do interpolated points later
		float offset = ((float)rand()/(float)RAND_MAX) * (radius / (1/waviness)) - (radius / (2/waviness));
		float x = radius * glm::cos(glm::radians(i * 360.f / num_points)) + offset;
		offset = ((float)rand() / (float)RAND_MAX) * (radius / (1/waviness)) - (radius / (2/waviness));
		float z = radius * glm::sin(glm::radians(i * 360.f / num_points)) + offset;
		float y = 0;
		control_points[i] = glm::vec3(x, y, -z);
	}
	// Interpolated points as midpoints
	for (int i = 0; i < num_points; i += 3)
		control_points[i] = 0.5f * (control_points[i - 1 > 0 ? i - 1 : num_points - 1] + control_points[i + 1]);

	// Calculate vertices
	bez_plane->vertices.push_back(glm::vec3(0.f)); // centered at origin
	for (unsigned int i = 0; i < num_curves; ++i)
	{
		int off = 3 * i;
		glm::mat4 bezier_mat = Util::calc_bezier_mat(control_points[off],
			control_points[off + 1],
			control_points[off + 2],
			control_points[(off + 3) % num_points]);

		for (unsigned int j = 0; j <= segmentation; ++j)
		{
			float t = (float) j / (float) segmentation;
			bez_plane->vertices.push_back(glm::vec3(bezier_mat * glm::vec4(t*t*t, t*t, t, 1.f)));
		}
	}

	// Normals
	for (int i = 0; i < bez_plane->vertices.size(); ++i)
		bez_plane->normals.push_back(glm::vec3(0.f, 1.f, 0.f));

	// Indices
	for (int i = 0; i < bez_plane->vertices.size(); ++i)
		bez_plane->indices.push_back(i);

	bez_plane->populate_buffers();
	geometries.push_back(bez_plane);
	return bez_plane;
}


