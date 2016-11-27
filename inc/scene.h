#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "scene_group.h"
#include "scene_camera.h"

class Scene
{
public:
	SceneGroup *root;
	SceneCamera *camera;
	glm::mat4 P;

	Scene();
	~Scene();
	void render();
};

