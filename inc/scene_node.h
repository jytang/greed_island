#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class SceneNode
{
public:
	virtual void draw(glm::mat4 m) = 0;
	virtual void update() = 0;
};

