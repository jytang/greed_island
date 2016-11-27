#include "scene_camera.h"

SceneCamera::SceneCamera(Scene *scene)
{
	this->scene = scene;
	// Default camera
	cam_pos = glm::vec3(0.0f, 10.0f, 20.0f);
	cam_look_at = glm::vec3(0.0f, 0.0f, 0.0f);
	cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
	V = glm::lookAt(cam_pos, cam_look_at, cam_up);
}

SceneCamera::~SceneCamera() {}

void SceneCamera::draw(glm::mat4 m)
{
}

void SceneCamera::update()
{
}
