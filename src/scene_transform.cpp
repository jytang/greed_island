#include "scene_transform.h"

SceneTransform::SceneTransform(glm::mat4 m)
{
	transformation = m;
}

SceneTransform::~SceneTransform() {}

void SceneTransform::draw(glm::mat4 m)
{
	glm::mat4 new_mat = m * transformation;
	SceneGroup::draw(new_mat);
}

void SceneTransform::update()
{

}