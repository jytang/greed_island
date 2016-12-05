#include "tree.h"

SceneModel *Tree::branch_model;
SceneModel *Tree::leaf_model;
Mesh Tree::branch_mesh;
Mesh Tree::leaf_mesh;
unsigned int Tree::leaf_layers;

SceneTransform *Tree::translate;
SceneTransform *Tree::rotate;
SceneTransform *Tree::scale;

GLfloat angle_delta;
GLfloat geo_size;

SceneGroup *Tree::generate_tree(Scene *scene, Geometry *base_branch, Geometry *base_leaf, unsigned int num_iterations, unsigned int leaves, GLfloat angle, GLfloat size, Material branch_material, Material leaf_material, int seed = 0)
{
	angle_delta = angle;
	leaf_layers = leaves;
	geo_size = size;
	SceneGroup *tree_group = new SceneGroup(scene);

	//Set Random Seed
	if (seed != 0)
		srand(seed);

	//Set Basic Properties
	branch_mesh = { base_branch, branch_material, ShaderManager::get_default(), glm::mat4(1.0f)};
	leaf_mesh = { base_leaf, leaf_material, ShaderManager::get_default() , glm::mat4(1.0f) };
	branch_model = new SceneModel(scene);
	leaf_model = new SceneModel(scene);

	tree_group->add_child(branch_model); //Base of tree BUT IT'S NOT SCALED RIGHT
	tree_group->add_child(leaf_model);
	
	tree_system(glm::mat4(1.0f), glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 0.0f, 1.f), 0.f, 0.f, 10.f, 1, num_iterations);

	branch_model->combine_meshes();
	leaf_model->combine_meshes();

	return tree_group;
}

void Tree::tree_system(glm::mat4 last_trans, glm::vec3 last_dir, glm::vec3 z_dir, float z_angle, float y_angle, float last_scale, unsigned int curr_iter, unsigned int max_iter)
{
	if (curr_iter > max_iter)
		return;

	float r, scale_value_y;

	//Handle Transformations
	r = (rand() % 2 == 0) ? -1.f : 1.f; //Positive or Negative Angle Change
	r *= (float)(rand() % 101 / 100.f);
	//fprintf(stderr, "Random: %f\n", r);	
	if (curr_iter == 0) //Most of the random stuff is kinda just me messing around with numbers.
	{
		z_angle = 0.f;
		scale_value_y = last_scale;
	}
	if (curr_iter == 2)
	{
		z_angle += (r*4.f);
	}
	else if (curr_iter < 4)
	{
		z_angle += (r * 1.f);
		y_angle += (r * 1.f);
	}
	else
	{
		z_angle += (r * 10.f);
		y_angle += (r * 10.f);
	}
	scale_value_y = last_scale * 0.8;

	glm::mat4 rot_mat, trans_mat, scale_mat;

	//Rotates around z-axis
	rot_mat = glm::rotate(glm::mat4(1.f), (z_angle * 2 * glm::pi<float>()) / 180.f, z_dir);

	//Rotates around y-axis
	rot_mat = glm::rotate(glm::mat4(1.f), (y_angle * 2 * glm::pi<float>()) / 180.f, last_dir) * rot_mat;

	glm::vec3 dir = glm::vec3(rot_mat * glm::vec4(0.0f, geo_size * scale_value_y, 0.0f, 1.0f)); //Maybe should divide by fourth element instead	

	trans_mat = last_trans;

	scale_mat = glm::scale(glm::mat4(1.f), glm::vec3(scale_value_y));

	glm::mat4 combined_mat = trans_mat * rot_mat;
	combined_mat = combined_mat * scale_mat;

	trans_mat = glm::translate(glm::mat4(1.f), dir) * last_trans;

	z_dir = glm::vec3(combined_mat * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	Mesh mesh;
	if (curr_iter < max_iter-leaf_layers)
	{
		mesh = branch_mesh;
		mesh.to_world = combined_mat;
		branch_model->add_mesh(mesh);
		tree_system(trans_mat, dir, z_dir, z_angle + angle_delta, y_angle, scale_value_y, curr_iter + 1, max_iter);
		tree_system(trans_mat, dir, z_dir, z_angle + angle_delta, y_angle + 130.f, scale_value_y, curr_iter + 1, max_iter);
		tree_system(trans_mat, dir, z_dir, z_angle + angle_delta, y_angle + 260.f, scale_value_y, curr_iter + 1, max_iter);
	}
	else
	{
		mesh = leaf_mesh;
		mesh.to_world = combined_mat;
		leaf_model->add_mesh(mesh);		
		tree_system(trans_mat, dir, z_dir, z_angle + angle_delta, y_angle, scale_value_y, curr_iter + 1, max_iter);
		tree_system(trans_mat, dir, z_dir, z_angle + angle_delta, y_angle + 130.f, scale_value_y, curr_iter + 2, max_iter);
		tree_system(trans_mat, dir, z_dir, z_angle + angle_delta, y_angle + 260.f, scale_value_y, curr_iter + 3, max_iter);
	}
}