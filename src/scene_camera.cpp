#include "scene_camera.h"

SceneCamera::SceneCamera(glm::mat4 m)
{
	view_matrix = m;
}

SceneCamera::~SceneCamera() {}

void SceneCamera::draw(glm::mat4 m)
{
}

void SceneCamera::update()
{
}
