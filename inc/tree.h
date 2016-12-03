#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <time.h>

#include "scene_model.h"
#include "scene_transform.h"
#include "colors.h"
#include "shader_manager.h"

class Tree
{
private:
	static void tree_system(glm::mat4, glm::vec3, glm::vec3, float, float, float last_scale, unsigned int curr_iter, unsigned int max_iter);
	
	static unsigned int leaf_threshold;
	//static SceneModel *tree_model;
	static SceneModel *branch_model;
	static SceneModel *leaf_model;
	static Mesh branch_mesh;
	static Mesh leaf_mesh;

	static SceneTransform *translate;
	static SceneTransform *rotate;
	static SceneTransform *scale;

public:
	static SceneGroup *generate_tree(Scene *, Geometry *, Geometry *, unsigned int, int);
};

