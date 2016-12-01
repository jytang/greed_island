#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "scene_camera.h"
#include "scene_group.h"
#include "shader.h"

class Scene
{
public:
	SceneGroup *root;
	SceneCamera *camera;
	glm::mat4 P;

	Scene();
	~Scene();
	void render();
	void pass(Shader * s);
};

