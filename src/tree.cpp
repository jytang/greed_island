#include "tree.h"

SceneModel *Tree::branch_model;
SceneModel *Tree::leaf_model;
Mesh Tree::branch_mesh;
Mesh Tree::leaf_mesh;
unsigned int Tree::leaf_threshold = 7;

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
	branch_material.diffuse = branch_material.ambient = color::brown;
	leaf_material.diffuse = leaf_material.ambient = color::windwaker_green;	
	branch_mesh = { base, branch_material, ShaderManager::get_default(), glm::mat4(1.0f)};
	leaf_mesh = { base, leaf_material, ShaderManager::get_default() , glm::mat4(1.0f) };
	branch_model = new SceneModel(scene);
	leaf_model = new SceneModel(scene);

	tree_group->add_child(branch_model); //Base of tree BUT IT'S NOT SCALED RIGHT
	tree_group->add_child(leaf_model);
	
	tree_system(glm::mat4(1.0f), 0.1f, 10.f, 1, num_iterations);

	branch_model->combine_meshes();
	leaf_model->combine_meshes();

	return tree_group;
}

void Tree::tree_system(glm::mat4 last_trans, float last_angle, float last_scale, unsigned int curr_iter, unsigned int max_iter)
{
	if (curr_iter > max_iter)
		return;

	float r, angle, scale_value;


	//Handle Transformations
	r = (rand() % 2 == 0) ? -1.f : 1.f; //Positive or Negative Angle Change
	r *= (float)(rand() % 101 / 100.f);
	//fprintf(stderr, "Random: %f\n", r);
	if (curr_iter == 0)
	{
		angle = 0.f;
		scale_value = last_scale;
	}
	if (curr_iter == 2)
	{
		angle = last_angle + (r*15.f);
		scale_value = last_scale * 0.3;
	}
	else if (curr_iter < 4)
	{
		angle = last_angle + (r * 1.f);
		scale_value = last_scale * 0.8f;
	}
	else
	{
		angle = last_angle + (r * 15.f);
		scale_value = last_scale * 0.9;
	}
	//float angle = 0;
	//angle = last_angle;
	//scale_value = last_scale * 0.8f;

	glm::mat4 rot_mat, trans_mat, scale_mat;

	rot_mat = glm::rotate(glm::mat4(1.f), (angle * 2 * glm::pi<float>()) / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec3 dir = glm::vec3(rot_mat * glm::vec4(0.0f, 4.f * scale_value, 0.0f, 1.0f)); //Maybe should divide by fourth element instead
																						 //glm::vec3 dir = glm::vec3(0.0f, last_scale + scale_value, 0.0f);

	trans_mat = last_trans;
	//glm::mat4 trans_mat = glm::translate(glm::mat4(1.f), dir) * last_trans;
	if (curr_iter < 4)
		scale_mat = glm::scale(glm::mat4(1.f), glm::vec3(scale_value * 0.8f, scale_value, scale_value* 0.8f));
	else
		scale_mat = glm::scale(glm::mat4(1.f), glm::vec3(scale_value));

	glm::mat4 combined_mat = trans_mat * rot_mat;
	combined_mat = combined_mat * scale_mat;

	trans_mat = glm::translate(glm::mat4(1.f), dir) * last_trans;


	Mesh mesh;
	if (curr_iter < leaf_threshold)
	{
		mesh = branch_mesh;
		mesh.to_world = combined_mat;
		branch_model->add_mesh(mesh);
		tree_system(trans_mat, angle + 15, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle - 15, scale_value, curr_iter + 1, max_iter);
		//tree_system(trans_mat, angle - 10, scale_value, curr_iter + 1, max_iter);
		//tree_system(trans_mat, angle + 10, scale_value, curr_iter + 1, max_iter);
	}
	else
	{
		mesh = leaf_mesh;
		mesh.to_world = combined_mat;
		leaf_model->add_mesh(mesh);
		//tree_system(trans_mat, angle + 20, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle + 10, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle + 5, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle - 5, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle - 10, scale_value, curr_iter + 1, max_iter);
		//tree_system(trans_mat, angle - 20, scale_value, curr_iter + 1, max_iter);
	}
}

/* Working 2D Fractal Tree
//Uses an iterated function system (IFS)
void Tree::tree_system(glm::mat4 last_trans, float last_angle, float last_scale, unsigned int curr_iter, unsigned int max_iter)
{
	if (curr_iter > max_iter)
		return;

	float r, angle, scale_value;
	
	
	//Handle Transformations
	r = (rand() % 2 == 0) ? -1.f : 1.f; //Positive or Negative Angle Change
	r *= (float)(rand() % 101 / 100.f);
	//fprintf(stderr, "Random: %f\n", r);
	if (curr_iter == 0)
	{
		angle = 0.f;
		scale_value = last_scale;
	}
	if (curr_iter == 2)
	{
		angle = last_angle + (r*15.f);
		scale_value = last_scale * 0.3;
	}
	else if (curr_iter < 4)
	{
		angle = last_angle + (r * 1.f);
		scale_value = last_scale * 0.8f;
	}	
	else
	{
		angle = last_angle + (r * 15.f);
		scale_value = last_scale * 0.9;
	}	
	//float angle = 0;
	//angle = last_angle;
	//scale_value = last_scale * 0.8f;

	glm::mat4 rot_mat, trans_mat, scale_mat;

	rot_mat = glm::rotate(glm::mat4(1.f), (angle * 2 * glm::pi<float>()) / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec3 dir = glm::vec3(rot_mat * glm::vec4(0.0f, 4.f * scale_value, 0.0f, 1.0f)); //Maybe should divide by fourth element instead
	//glm::vec3 dir = glm::vec3(0.0f, last_scale + scale_value, 0.0f);
	
	trans_mat = last_trans;
	//glm::mat4 trans_mat = glm::translate(glm::mat4(1.f), dir) * last_trans;
	if(curr_iter < 4)
		scale_mat = glm::scale(glm::mat4(1.f), glm::vec3(scale_value * 0.8f, scale_value, scale_value* 0.8f));
	else
		scale_mat = glm::scale(glm::mat4(1.f), glm::vec3(scale_value));

	glm::mat4 combined_mat = trans_mat * rot_mat;
	combined_mat = combined_mat * scale_mat;	

	trans_mat = glm::translate(glm::mat4(1.f), dir) * last_trans;

	
	Mesh mesh;
	if (curr_iter < leaf_threshold)
	{
		mesh = branch_mesh;
		mesh.to_world = combined_mat;
		branch_model->add_mesh(mesh);
		tree_system(trans_mat, angle + 15, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle - 15, scale_value, curr_iter + 1, max_iter);
		//tree_system(trans_mat, angle - 10, scale_value, curr_iter + 1, max_iter);
		//tree_system(trans_mat, angle + 10, scale_value, curr_iter + 1, max_iter);
	}
	else
	{
		mesh = leaf_mesh;
		mesh.to_world = combined_mat;
		leaf_model->add_mesh(mesh);
		//tree_system(trans_mat, angle + 20, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle + 10, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle + 5, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle - 5, scale_value, curr_iter + 1, max_iter);
		tree_system(trans_mat, angle - 10, scale_value, curr_iter + 1, max_iter);
		//tree_system(trans_mat, angle - 20, scale_value, curr_iter + 1, max_iter);
	}	
}
*/