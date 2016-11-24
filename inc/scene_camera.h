#pragma once

#include "scene_node.h"

class SceneCamera :
	public SceneNode
{
public:
	glm::mat4 view_matrix;

	SceneCamera(glm::mat4 m);
	~SceneCamera();
	void draw(glm::mat4 m);
	void update();
};

