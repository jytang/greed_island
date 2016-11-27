#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class Scene;

class SceneNode
{
protected:
	Scene *scene;
public:
	virtual void draw(glm::mat4 m) = 0;
	virtual void update() = 0;
};

