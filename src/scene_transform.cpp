#include "scene_transform.h"

SceneTransform::SceneTransform(Scene *scene, glm::mat4 m)
{
	this->scene = scene;
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