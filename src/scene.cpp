#include "scene.h"

Scene::Scene()
{
	root = new SceneGroup(this);
	camera = new SceneCamera(this);
	light_pos = glm::vec3(0.f, 2.f, 1.f);
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