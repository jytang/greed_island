#include "geometry_generator.h"

GeometryGenerator::GeometryGenerator() {}

GeometryGenerator::~GeometryGenerator() {}

Geometry * GeometryGenerator::generate_cube(GLfloat scale)
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
	cube->normals.push_back(glm::normalize(v0));
	cube->vertices.push_back(v1);
	cube->normals.push_back(glm::normalize(v1));
	cube->vertices.push_back(v2);
	cube->normals.push_back(glm::normalize(v2));
	cube->vertices.push_back(v3);
	cube->normals.push_back(glm::normalize(v3));
	cube->vertices.push_back(v4);
	cube->normals.push_back(glm::normalize(v4));
	cube->vertices.push_back(v5);
	cube->normals.push_back(glm::normalize(v5));
	cube->vertices.push_back(v6);
	cube->normals.push_back(glm::normalize(v6));
	cube->vertices.push_back(v7);
	cube->normals.push_back(glm::normalize(v7));

	// TOP
	cube->indices.push_back(0);
	cube->indices.push_back(2);
	cube->indices.push_back(1);
	cube->indices.push_back(1);
	cube->indices.push_back(2);
	cube->indices.push_back(3);

	// FRONT
	cube->indices.push_back(2);
	cube->indices.push_back(6);
	cube->indices.push_back(3);
	cube->indices.push_back(3);
	cube->indices.push_back(6);
	cube->indices.push_back(7);
	
	// LEFT
	cube->indices.push_back(1);
	cube->indices.push_back(3);
	cube->indices.push_back(5);
	cube->indices.push_back(3);
	cube->indices.push_back(7);
	cube->indices.push_back(5);
	
	// RIGHT
	cube->indices.push_back(0);
	cube->indices.push_back(4);
	cube->indices.push_back(2);
	cube->indices.push_back(2);
	cube->indices.push_back(4);
	cube->indices.push_back(6);
	
	// BACK
	cube->indices.push_back(1);
	cube->indices.push_back(5);
	cube->indices.push_back(0);
	cube->indices.push_back(0);
	cube->indices.push_back(5);
	cube->indices.push_back(4);
	
	// BOTTOM
	cube->indices.push_back(5);
	cube->indices.push_back(7);
	cube->indices.push_back(4);
	cube->indices.push_back(4);
	cube->indices.push_back(7);
	cube->indices.push_back(6);

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
	return nullptr;
}
