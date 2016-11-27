#include "scene_model.h"

SceneModel::SceneModel(Scene *scene)
{
	this->scene = scene;
}

SceneModel::~SceneModel() {}

void SceneModel::add_mesh(Mesh m)
{
	meshes.push_back(m);
}

void SceneModel::draw(glm::mat4 m)
{
	// Loop over meshes and their respective shader programs.
	for (Mesh mesh : meshes)
	{

	}
}

void SceneModel::update()
{

}