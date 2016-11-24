#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <list>

#include "scene_node.h"

class SceneGroup :
	public SceneNode
{
protected:
	std::list<SceneNode *> children;
public:
	SceneGroup();
	~SceneGroup();
	void add_child(SceneNode *node);
	void remove_child(SceneNode *node);
	virtual void draw(glm::mat4 m);
	virtual void update();
};

