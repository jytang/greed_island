#include "tree.h"

SceneModel *Tree::tree_model;
Mesh Tree::branch_mesh;
Mesh Tree::leaf_mesh;
unsigned int Tree::leaf_threshold = 10;

SceneTransform *Tree::translate;
SceneTransform *Tree::rotate;
SceneTransform *Tree::scale;

SceneModel *Tree::generate_tree(Scene *scene, Geometry *base, unsigned int num_iterations, int seed = 0)
{	
	//SceneGroup *tree_group = new SceneGroup(scene);

	//Set Random Seed
	if (seed == 0)
		srand((unsigned int)time(NULL));
	else
		srand(seed);

	//Set Basic Properties
	Material branch_material, leaf_material;
	branch_material.diffuse = branch_material.ambient = color::windwaker_sand;
	leaf_material.diffuse = leaf_material.ambient = color::windwaker_green;	
	branch_mesh = { base, branch_material, ShaderManager::get_default(), glm::mat4(1.0f)};
	leaf_mesh = { base, leaf_material, ShaderManager::get_default() , glm::mat4(1.0f) };
	tree_model = new SceneModel(scene);

	//tree_group->add_child(branch_model); //Base of tree BUT IT'S NOT SCALED RIGHT
	
	tree_system(glm::mat4(1.0f), 0.f, 10.f, 1, num_iterations);

	return tree_model;
}

//Uses an iterated function system (IFS)
void Tree::tree_system(glm::mat4 last_trans, float last_angle, float last_scale, unsigned int curr_iter, unsigned int max_iter)
{
	if (curr_iter > max_iter)
		return;

	//Recursively scale, rotate, and translate base, and then add to current scene group. 
	//Color changes after a certain iteration.
	//Assume base is of size 1x1x1 for now
	
	//Handle Transformations
	float r = (rand() % 2 == 0) ? -1.f : 1.f; //Positive or Negative Angle Change
	r *= (float)(rand() % 101 / 100.f);
	//fprintf(stderr, "Random: %f\n", r);
	float angle = (r * 20.0f);
	float scale_value = last_scale * 0.6f;	

	glm::mat4 rot_mat = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec3 dir = glm::vec3(rot_mat * glm::vec4(0.0f, scale_value * 2, 0.0f, 1.0f)); //Maybe should divide by fourth element instead
	
	glm::mat4 trans_mat = glm::translate(glm::mat4(1.f), dir) * last_trans;
	glm::mat4 scale_mat = glm::scale(glm::mat4(1.f), glm::vec3(scale_value));

	glm::mat4 combined_mat = trans_mat * rot_mat;
	combined_mat = combined_mat * scale_mat;

	//SceneTransform *transformation = new SceneTransform(scene, combined_mat);
	
	Mesh mesh;
	if (curr_iter < leaf_threshold)
		mesh = branch_mesh;
	else
		mesh = leaf_mesh;	
	mesh.to_world = combined_mat;	
	tree_model->add_mesh(mesh);
	

	/*
	//Attach Transformations
	if (curr_iter < leaf_threshold)
		transformation->add_child(branch_model);
	else
		transformation->add_child(leaf_model);	
	//rotate->add_child(scale);
	//translate->add_child(rotate);
	last_group->add_child(transformation);	
	*/

	tree_system(trans_mat, angle, scale_value, curr_iter + 1, max_iter);
	tree_system(trans_mat, angle, scale_value, curr_iter + 1, max_iter);
}