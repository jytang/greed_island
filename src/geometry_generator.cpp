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
