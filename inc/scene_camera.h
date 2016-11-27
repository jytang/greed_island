#pragma once

#include "scene_node.h"
#include <glm/gtc/matrix_transform.hpp>

class SceneCamera :
	public SceneNode
{
public:
	glm::vec3 cam_pos;
	glm::vec3 cam_look_at;
	glm::vec3 cam_up;
	glm::mat4 V;

	SceneCamera(Scene *);
	~SceneCamera();
	void draw(glm::mat4 m);
	void update();
};

