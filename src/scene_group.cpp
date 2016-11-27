#include "scene_group.h"

SceneGroup::SceneGroup() {}

SceneGroup::SceneGroup(Scene *scene)
{
	this->scene = scene;
}

SceneGroup::~SceneGroup()
{
	for (auto it = children.begin(); it != children.end(); ++it)
		delete(*it);
}

void SceneGroup::add_child(SceneNode *node)
{
	children.push_back(node);
}

void SceneGroup::remove_child(SceneNode *node)
{
	children.remove(node);
}

void SceneGroup::draw(glm::mat4 m)
{
	for (auto it = children.begin(); it != children.end(); ++it)
		(*it)->draw(m);
}

void SceneGroup::update()
{
	for (auto it = children.begin(); it != children.end(); ++it)
		(*it)->update();
}
