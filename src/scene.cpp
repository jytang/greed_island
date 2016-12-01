#include "scene.h"

Scene::Scene()
{
	root = new SceneGroup(this);
	camera = new SceneCamera(this);
}

Scene::~Scene()
{
	delete(root);
	delete(camera);
}

void Scene::render()
{
	root->draw(glm::mat4(1.f));
}

void Scene::pass(Shader * s)
{
	root->pass(glm::mat4(1.f), s);
}