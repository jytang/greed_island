#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "scene_camera.h"
#include "scene_group.h"
#include "shader.h"
#include "plane.h"

class Scene
{
public:
	SceneGroup *root;
	SceneCamera *camera;
	glm::mat4 P;
	glm::vec3 light_pos;
	Plane frustum_planes[6];
	glm::vec3 frustum_corners[8];

	Scene();
	~Scene();
	void render();
	void pass(Shader * s);
	void update_frustum_corners(int width, int height, GLfloat);
	void update_frustum_planes();
	glm::mat4 frustum_ortho();
};

