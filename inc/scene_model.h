#pragma once
#include "scene_node.h"
#include "mesh.h"

#include <vector>

class SceneModel :
	public SceneNode
{
public:
	std::vector<Mesh> meshes;
	glm::mat4 model_mat;

	SceneModel();
	~SceneModel();
	void add_mesh(Mesh m);
	void draw(glm::mat4);
	void update();
};

