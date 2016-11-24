#include "scene_model.h"

SceneModel::SceneModel() {}

SceneModel::~SceneModel() {}

void SceneModel::add_mesh(Mesh m)
{
	meshes.push_back(m);
}

void SceneModel::draw(glm::mat4 m)
{
	// Loop over meshes and their respective shader programs.
}

void SceneModel::update()
{

}