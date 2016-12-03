#include "tree.h"

SceneModel *Tree::branch_model;
SceneModel *Tree::leaf_model;
unsigned int Tree::leaf_threshold = 10;

SceneTransform *Tree::translate;
SceneTransform *Tree::rotate;
SceneTransform *Tree::scale;

SceneGroup *Tree::generate_tree(Scene *scene, Geometry *base, unsigned int num_iterations, int seed = 0)
{	
	SceneGroup *tree_group = new SceneGroup(scene);

	//Set Random Seed
	if (seed == 0)
		srand((unsigned int)time(NULL));
	else
		srand(seed);

	//Set Basic Properties
	Material branch_material, leaf_material;
	branch_material.diffuse = branch_material.ambient = color::windwaker_sand;
	leaf_material.diffuse = leaf_material.ambient = color::windwaker_green;	
	Mesh branch_mesh = { base, branch_material, ShaderManager::get_default() };
	Mesh leaf_mesh = { base, leaf_material, ShaderManager::get_default() };
	branch_model = new SceneModel(scene);
	branch_model->add_mesh(branch_mesh);
	leaf_model = new SceneModel(scene);
	leaf_model->add_mesh(leaf_mesh);	

	tree_group->add_child(branch_model); //Base of tree
	
	tree_system(scene, tree_group, 0.f, 10.f, 1, num_iterations);

	return tree_group;
}

//Uses an iterated function system (IFS)
void Tree::tree_system(Scene *scene, SceneGroup *last_group, float last_angle, float last_scale, unsigned int curr_iter, unsigned int max_iter)
{
	if (curr_iter > max_iter)
		return;

	//Recursively scale, rotate, and translate base, and then add to current scene group. 
	//Color changes after a certain iteration.
	//Assume base is of size 1x1x1 for now
	
	//Handle Transformations
	float r = (float) (rand() % 2 == 0) ? -1 : 1; //Positive or Negative Angle Change
	r = r * (float)(rand() % 101 / 100.f);
	fprintf(stderr, "Random: %f\n", r);
	float angle = (r * 45.0f);
	float scale_value = last_scale * 0.95f;	

	glm::mat4 rot_mat = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec3 dir = glm::vec3(rot_mat * glm::vec4(0.0f, scale_value * 2, 0.0f, 1.0f)); //Maybe should divide by fourth element instead
	
	translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), dir));
	rotate = new SceneTransform(scene, rot_mat);
	scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(scale_value)));

	//Attach Transformations
	if (curr_iter < leaf_threshold)
		scale->add_child(branch_model);
	else
		scale->add_child(leaf_model);	
	rotate->add_child(scale);
	translate->add_child(rotate);
	last_group->add_child(translate);

	//fprintf(stderr, "Adding Transforms for iteration %u\n", curr_iter);

	/*

	float angle = 0;
	float scale_value = 1.0f;

	translate = new SceneTransform(scene, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

	translate->add_child(branch_model);
	last_group->add_child(translate);
	*/

	tree_system(scene, translate, angle, scale_value, curr_iter + 1, max_iter);
	tree_system(scene, translate, angle, scale_value, curr_iter + 1, max_iter);
}