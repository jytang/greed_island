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

	// TOP
	// FRONT

	// LEFT

	// RIGHT

	// BACK

	// BOTTOM

	if (has_normals)
	{
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(0.f, 1.f, 0.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(0.f, 0.f, 1.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(-1.f, 0.f, 0.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(1.f, 0.f, 0.f));
		for (int i = 0; i < 6; ++i)
			cube->normals.push_back(glm::vec3(0.f, 0.f, -1.f));
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

	plane->has_normals = true;
	plane->populate_buffers();
	geometries.push_back(plane);

	return plane;
}

/*
Geometry * GeometryGenerator::generate_patch(GLfloat scale)
{

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

	bez_plane->has_normals = true;
	bez_plane->populate_buffers();
	geometries.push_back(bez_plane);
	return bez_plane;
}