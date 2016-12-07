#include "greed.h"
#include "window.h"
#include "skybox_shader.h"
#include "shadow_shader.h"
#include "geometry_generator.h"
#include "scene_model.h"
#include "scene_transform.h"
#include "scene_animation.h"
#include "tree.h"
#include "shape_grammar.h"
#include <cfloat>

#include "util.h"
#include "colors.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* global vars */
vr_vars GreedVR::vars;
Scene* scene;
Scene* island_scene;
Scene* desert_scene;
Scene* transition_scene;
std::vector<Scene*> scenes;
SceneCamera* camera;
SceneGroup *forest;
SceneTransform *map;
SceneGroup *village;

Geometry *cylinder_geo;
Geometry *diamond_geo;
Geometry *sphere_geo;
Geometry *cube_geo;
Geometry *land_geo;
Geometry *plateau_geo;
Geometry *sand_geo;

bool keys[1024];
bool lmb_down = false;
bool rmb_down = false;
bool debug_shadows = false;
bool mouse_moved = false;
bool shadows_on = true;
glm::vec3 last_cursor_pos;

const GLfloat FAR_PLANE = 1000.f;
const GLfloat FOV = 45.f;
const GLfloat ISLAND_SIZE = 600.f;
const GLfloat PLAYER_HEIGHT = 1.f;
// Procedural generation parameters
const GLuint    HEIGHT_MAP_POWER = 8;
const GLuint    HEIGHT_MAP_SIZE = (unsigned int)glm::pow(2, HEIGHT_MAP_POWER) + 1;
const GLfloat   HEIGHT_MAP_MAX = 200.f;
const GLfloat   HEIGHT_RANDOMNESS_SCALE = 200.f;
const GLint     VILLAGE_DIAMETER = 60;
const GLfloat   TERRAIN_SIZE = ISLAND_SIZE / 5;
const GLfloat   TERRAIN_SCALE = ISLAND_SIZE / 60;
const GLuint    TERRAIN_RESOLUTION = 200;
const GLfloat   BEACH_HEIGHT = 10.f;
const GLuint    NUM_TREES = 200;
const GLfloat   PERCENT_TREE_ANIM = 0.9f;
const GLuint    NUM_TREE_TYPES = 10;
const GLfloat   TREE_SCALE = 1.5f;
const GLfloat   PATH_WIDTH = 90.f;
const GLfloat   FOREST_RADIUS = ISLAND_SIZE / 1.1f;
const GLfloat   FOREST_INNER_CIRCLE = VILLAGE_DIAMETER*TERRAIN_SCALE / 3;
const GLint		NUM_BUILDINGS = 5;
const GLfloat	VILLAGE_DIAMETER_TRUE = ((float)VILLAGE_DIAMETER / HEIGHT_MAP_SIZE) * TERRAIN_SIZE * TERRAIN_SCALE;
const GLfloat   WATER_SCALE = ISLAND_SIZE * 4;
const GLint     CAM_OFFSET = 20;

bool vr_on = false;
SceneTransform *controller_1_translate;
SceneTransform *controller_2_translate;

Greed::Greed() {}

Greed::~Greed() {}

void Greed::destroy()
{
	// Free memory here.
	//delete(transition_scene);
	delete(island_scene);
	ShaderManager::destroy();
	GeometryGenerator::clean_up();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Greed::next_skybox()
{
	SkyboxShader * ss = ((SkyboxShader *)ShaderManager::get_shader_program("skybox"));
	ss->current_texture_id = (ss->current_texture_id + 1) % ss->texture_ids.size();
}

void Greed::change_scene(Scene * s)
{
	scene = s;
	camera = s->camera;
}

void Greed::next_scene()
{
	int curr = 0;
	for (auto it = scenes.begin(); it != scenes.end(); ++it)
	{
		if (*it == scene)
			curr = (int) (it - scenes.begin());
	}
	next_skybox();
	change_scene(scenes[(curr + 1) % scenes.size()]);
}

void Greed::setup_shaders()
{
	// Load shaders via a shader manager.
	ShaderManager::create_shader_program("basic");
	ShaderManager::create_shader_program("skybox");
	ShaderManager::create_shader_program("shadow");
	ShaderManager::create_shader_program("debug_shadow");
	ShaderManager::set_default("basic");
}

void Greed::generate_forest()
{
	std::cerr << "Generating Forest" << std::endl;

	if (!forest)
	{
		forest = new SceneGroup(scene);
		scene->root->add_child(forest);
	}
	else {
		forest->remove_all();
	}

	glm::vec3 leaf_colors[] = { color::olive_green, color::olive_green, color::olive_green, color::autumn_orange, color::purple, color::bone_white, color::indian_red };
	glm::vec3 branch_colors[] = { color::brown, color::wood_saddle, color::wood_sienna, color::wood_tan, color::wood_tan_light };
	Material branch_material, leaf_material;
	for (int i = 0; i < NUM_TREES; ++i) {
		leaf_material.diffuse = leaf_material.ambient = leaf_colors[(int)Util::random(0, 7)];
		branch_material.diffuse = branch_material.ambient = branch_colors[(int)Util::random(0, 5)];
		bool animated = false;
		if (i % (NUM_TREES / (int)(NUM_TREES*PERCENT_TREE_ANIM)) == 0)
			animated = true;
		if (i % 50 == 0)
			std::cerr << "Tree " << i << std::endl;
		float x, z;
		do {
			float angle = Util::random(0, 360);
			float distance = Util::random(FOREST_INNER_CIRCLE, FOREST_RADIUS);

			x = glm::cos(glm::radians(angle)) * distance;
			z = glm::sin(glm::radians(angle)) * distance;
		} while (Util::within_rect(glm::vec2(x, z), glm::vec2(-PATH_WIDTH / 2, FOREST_RADIUS), glm::vec2(PATH_WIDTH / 2, 0)));
		float y = Terrain::height_lookup(x, z, ISLAND_SIZE * 2);
		glm::vec3 location = { x, y, z };
		SceneGroup *tree = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 7, 1, 20.f, 2.f, branch_material, leaf_material, animated, location, 0);
		((SceneModel *)(tree->children[0]))->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		if (animated)
			((SceneModel *)((SceneAnimation *)(tree->children[1]))->children[0])->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		else
			((SceneModel *)(tree->children[1]))->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		forest->add_child(tree);
	}
	std::cerr << "Done." << std::endl;
}

void Greed::generate_map()
{
	std::cerr << "Generating Map" << std::endl;

	if (!map)
	{
		map = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(TERRAIN_SCALE, 1.f, TERRAIN_SCALE)));
		scene->root->add_child(map);
	}
	else {
		map->remove_all();
	}

	Terrain::generate_height_map(HEIGHT_MAP_SIZE, HEIGHT_MAP_MAX, VILLAGE_DIAMETER, HEIGHT_RANDOMNESS_SCALE, 0);
	float cam_height = Terrain::height_lookup(0.f, ISLAND_SIZE - CAM_OFFSET, ISLAND_SIZE * 2);
	camera->cam_pos.y = cam_height + PLAYER_HEIGHT;
	camera->recalculate();

	std::cerr << "Generating Land" << std::endl;
	// Second Parameter Below is Resolution^2 of Island, LOWER TO RUN FASTER
	land_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, BEACH_HEIGHT, HEIGHT_MAP_MAX * 0.8f - 0.2f, false);
	Material land_material;
	land_material.diffuse = land_material.ambient = color::windwaker_green;
	Mesh land_mesh = { land_geo, land_material, ShaderManager::get_default(), glm::mat4(1.f) };

	std::cerr << "Generating Plateau" << std::endl;
	plateau_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, HEIGHT_MAP_MAX * 0.8f - 0.2f, HEIGHT_MAP_MAX, false);
	Material plateau_material;
	plateau_material.diffuse = plateau_material.ambient = color::bone_white;
	Mesh plateau_mesh = { plateau_geo, plateau_material, ShaderManager::get_default(), glm::mat4(1.f) };

	std::cerr << "Generating Sand" << std::endl;
	sand_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, 0.0f, BEACH_HEIGHT, true);
	Material sand_material;
	sand_material.diffuse = sand_material.ambient = color::windwaker_sand;
	Mesh sand_mesh = { sand_geo, sand_material, ShaderManager::get_default(), glm::mat4(1.f) };

	SceneModel *terrain_model = new SceneModel(scene);
	terrain_model->add_mesh(land_mesh);
	terrain_model->add_mesh(sand_mesh);
	terrain_model->add_mesh(plateau_mesh);
	map->add_child(terrain_model);
	std::cerr << "Done." << std::endl;
}

void Greed::generate_village()
{
	std::cerr << "Generating Village" << std::endl;

	if (!village)
	{
		village = new SceneGroup(scene);
		scene->root->add_child(village);
	}
	else {
		village->remove_all();
	}

	for (int i = 0; i < NUM_BUILDINGS; ++i)
	{
		std::cerr << "House " << i << std::endl;

		float angle = ((350.f / (NUM_BUILDINGS)) * i) - 60.f; //Circles around, starting from the right
		float distance = VILLAGE_DIAMETER_TRUE / 3.f;

		float x = glm::cos(glm::radians(angle)) * distance;
		float z = -glm::sin(glm::radians(angle)) * distance;

		float y = Terrain::height_lookup(x, z, ISLAND_SIZE * 2);
		glm::vec3 location = { x, y, z };

		float rot = glm::radians(-90.f + angle);

		SceneModel *building = ShapeGrammar::generate_building(scene, 0);
		SceneTransform *building_rotate = new SceneTransform(scene, glm::rotate(glm::mat4(1.f), rot, glm::vec3(0.f, 1.f, 0.f)));
		SceneTransform *building_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), location));
		building_rotate->add_child(building);
		building_translate->add_child(building_rotate);
		village->add_child(building_translate);
	}
	std::cerr << "Done." << std::endl;
}

void Greed::generate_miniatures()
{
	std::cerr << "Generating Miniatures" << std::endl;
	SceneGroup *root = scene->root;

	// Small-scale showcase.
	const GLfloat SMALL_ROT_SPEED = 0.3f;
	// Small terrain
	Material small_land_material;
	small_land_material.diffuse = small_land_material.ambient = color::windwaker_green;
	small_land_material.shadows = false;
	Mesh small_land_mesh = { land_geo, small_land_material, ShaderManager::get_default(), glm::mat4(1.f) };
	Material small_plateau_material;
	small_plateau_material.diffuse = small_plateau_material.ambient = color::bone_white;
	small_plateau_material.shadows = false;
	Mesh small_plateau_mesh = { plateau_geo, small_plateau_material, ShaderManager::get_default(), glm::mat4(1.f) };
	Material small_sand_material;
	small_sand_material.diffuse = small_sand_material.ambient = color::windwaker_sand;
	small_sand_material.shadows = false;
	Mesh small_sand_mesh = { sand_geo, small_sand_material, ShaderManager::get_default(), glm::mat4(1.f) };
	SceneModel *small_terrain_model = new SceneModel(scene);
	small_terrain_model->add_mesh(small_land_mesh);
	small_terrain_model->add_mesh(small_sand_mesh);
	small_terrain_model->add_mesh(small_plateau_mesh);
	SceneTransform *small_terrain_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.05f / 10.f, 0.05f)));
	SceneAnimation *small_terrain_anim = new SceneAnimation(scene, 0.f, FLT_MAX, 0.f, SMALL_ROT_SPEED, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
	SceneTransform *small_terrain_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(80.f, 175.f, -40.f)));
	small_terrain_scale->add_child(small_terrain_model);
	small_terrain_anim->add_child(small_terrain_scale);
	small_terrain_translate->add_child(small_terrain_anim);
	root->add_child(small_terrain_translate);

	// Small trees
	Material branch_material, leaf_material;
	leaf_material.diffuse = leaf_material.ambient = color::olive_green;
	branch_material.diffuse = branch_material.ambient = color::wood_sienna;
	branch_material.shadows = false;
	leaf_material.shadows = false;
	SceneGroup *small_tree_1 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 2, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_2 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 3, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_3 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 4, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_4 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 5, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_5 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 6, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_6 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 7, 1, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_trees[6] = { small_tree_1, small_tree_2, small_tree_3, small_tree_4, small_tree_5, small_tree_6 };
	SceneGroup *small_forest = new SceneGroup();
	float SMALL_FOREST_RADIUS = 5.f;
	for (int i = 0; i < 6; ++i)
	{
		SceneTransform *small_tree_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(0.05f)));
		SceneAnimation *small_tree_anim = new SceneAnimation(scene, 0.f, FLT_MAX, 0.f, SMALL_ROT_SPEED, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		// Display in a half-circle arc.
		float x = SMALL_FOREST_RADIUS*glm::cos(glm::radians(180.f / 5.f * (5 - i)));
		float z = -SMALL_FOREST_RADIUS*glm::sin(glm::radians(180.f / 5.f * i));
		SceneTransform *small_tree_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(x, 0, z)));
		small_tree_scale->add_child(small_trees[i]);
		small_tree_anim->add_child(small_tree_scale);
		small_tree_translate->add_child(small_tree_anim);
		small_forest->add_child(small_tree_translate);
	}

	SceneTransform *small_forest_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(-80.f, 175.f, -40.f)));
	small_forest_translate->add_child(small_forest);
	root->add_child(small_forest_translate);
	std::cerr << "Done." << std::endl;
}

void Greed::setup_scene()
{
	island_scene = new Scene();
	transition_scene = new Scene();
	desert_scene = new Scene();
	scenes.push_back(island_scene);
	scenes.push_back(transition_scene);
	scenes.push_back(desert_scene);

	scene = island_scene;
	SceneGroup *root = scene->root;
	
	// Lights and camera before action.
	camera = scene->camera;
	// Set all cameras to be the same.
	for (Scene* s : scenes)
	{
		s->camera = camera;
	}
	// Initial position
	camera->cam_pos = glm::vec3(0.f, 0.f, ISLAND_SIZE - CAM_OFFSET);
	camera->recalculate();
	// Light(s)
	scene->light_pos = glm::vec3(0.f, ISLAND_SIZE*2, ISLAND_SIZE);
	
	/* BEGIN ACTUAL OBJECTS TO BE RENDERED */
	// "Starter" geometry kit
	cylinder_geo = GeometryGenerator::generate_cylinder(0.25f, 2.f, 3, false);
	diamond_geo = GeometryGenerator::generate_sphere(2.f, 3);
	sphere_geo = GeometryGenerator::generate_sphere(1.f, 7);
	cube_geo = GeometryGenerator::generate_cube(1.f, true);

	// Skybox
	Material default_material;
	Mesh skybox_mesh = { nullptr, default_material, ShaderManager::get_shader_program("skybox"), glm::mat4(1.f) };
	SceneModel *skybox_model = new SceneModel(scene);
	skybox_model->add_mesh(skybox_mesh);
	root->add_child(skybox_model);
	transition_scene->root->add_child(skybox_model);

	// Infinite water plane stretching to the horizon
	Geometry *plane_geo = GeometryGenerator::generate_plane(1.f);
	Material water_material;
	water_material.diffuse = water_material.ambient = color::ocean_blue;
	Mesh water_mesh = { plane_geo, water_material, ShaderManager::get_default(), glm::mat4(1.f) };
	SceneModel *water_model = new SceneModel(scene);
	water_model->add_mesh(water_mesh);
	SceneTransform *water_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(WATER_SCALE, 1.0f, WATER_SCALE)));
	SceneTransform *water_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -2.f, 0.0f)));
	water_scale->add_child(water_model);
	water_translate->add_child(water_scale);
	root->add_child(water_translate);

	// Curvy beach plane, named Bezier Beach Resort
	Geometry *beach_geo = GeometryGenerator::generate_bezier_plane(ISLAND_SIZE*1.5f, 50, 150, 0.1f, 0);
	Material beach_material;
	beach_material.diffuse = beach_material.ambient = color::windwaker_sand;
	Mesh beach_mesh = { beach_geo, beach_material, ShaderManager::get_default(), glm::mat4(1.f) };
	SceneModel *beach_model = new SceneModel(scene);
	beach_model->add_mesh(beach_mesh);
	SceneTransform *beach_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(1.0f, 1.0f, 1.0f)));
	SceneTransform *beach_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.1f, 0.0f)));
	beach_scale->add_child(beach_model);
	beach_translate->add_child(beach_scale);
	root->add_child(beach_translate);
	desert_scene->root->add_child(beach_translate);
	
	// Generate everything.
	generate_map();
	generate_forest();
	generate_village();
	generate_miniatures();

	//Show Vive Controllers using Two Blue Spheres
	if (vr_on)
	{
		Material sphere_material;
		sphere_material.diffuse = sphere_material.ambient = color::ocean_blue;
		Mesh sphere_mesh = { sphere_geo, sphere_material, ShaderManager::get_default() };
		SceneModel *sphere_model = new SceneModel(scene);
		sphere_model->add_mesh(sphere_mesh);
		SceneTransform *sphere_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(1.f, 1.0f, 1.f)));
		controller_1_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.f, 0.0f)));
		controller_2_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.f, 0.0f)));
		sphere_scale->add_child(sphere_model);
		controller_1_translate->add_child(sphere_scale);
		controller_2_translate->add_child(sphere_scale);
		root->add_child(controller_1_translate);
		root->add_child(controller_2_translate);
	}
}

void Greed::go()
{
	window = Window::create_window(1280, 720, "Greed Island");
	setup_callbacks();
	setup_opengl();
	if (vr_on) GreedVR::init();

	setup_shaders();
	// Seed PRNG.
	Util::seed(0);
	setup_scene();

	// Send height/width of window
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	resize_callback(window, width, height);

	GLuint frame = 0;
	double prev_ticks = glfwGetTime();
	double move_prev_ticks = prev_ticks;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		frame++;
		double curr_time = glfwGetTime();
		if (curr_time - prev_ticks > 1.f)
		{
			std::cerr << "FPS: " << frame << std::endl;
			frame = 0;
			prev_ticks = curr_time;
		}
		if (curr_time - move_prev_ticks > 1.f / 60.f)
		{
			if (vr_on)
				handle_movement_vr();					
			else
				handle_movement();
			move_prev_ticks = curr_time;
		}

		glfwGetFramebufferSize(window, &width, &height);
		scene->update_frustum_planes();
		scene->update_frustum_corners(width, height, FAR_PLANE);

		// Rotate sun.
		if (keys[GLFW_KEY_LEFT_ALT])
		{
			float sun_step = 0.005f;

			if (scene->light_pos.y < 10.f)
				sun_step *= 2;
			else
				sun_step *= glm::max(scene->light_pos.y / (ISLAND_SIZE * 2), 0.2f);
			
			sun_step *= scene->light_pos.y < 0 ? 3 : 1;
			scene->light_pos = glm::vec3(glm::rotate(glm::mat4(1.0f), sun_step, glm::vec3(0.f, 0.f, 1.f)) * glm::vec4(scene->light_pos, 1.0f));
		}

		// First pass: shadowmap.
		shadow_pass();

		// Second pass: usual rendering.
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (vr_on)
		{

			//Update Controller Positions
			//vr_update_controllers();

			//Update any objects relying on controller positions
				//Including Controller's Spheres and Grabbed Objects

			vr_render(); //Render Scene

			/*// Mirror to the window //CURRENTLY DOESN"T WORK
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBlitFramebuffer(0, 0, GreedVR::vars.framebufferWidth, GreedVR::vars.framebufferHeight, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);
			*/
		}
		else {
			scene->render();
			// Debug shadows.
			if (debug_shadows)
			{
				glViewport(0, 0, width / 3, height / 3);
				ShaderManager::get_shader_program("debug_shadow")->use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, ((ShadowShader *)ShaderManager::get_shader_program("shadow"))->shadow_map_tex);
				Util::render_quad();
			}
		}

		glfwSwapBuffers(window);
	}

	destroy();
}

void Greed::shadow_pass()
{
	ShadowShader * ss = (ShadowShader *) ShaderManager::get_shader_program("shadow");
	glViewport(0, 0, ss->size, ss->size);
	glBindFramebuffer(GL_FRAMEBUFFER, ss->FBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	ss->use();
	ss->light_pos = scene->light_pos;
	if (shadows_on)
		ss->light_proj = scene->frustum_ortho();//glm::ortho(-ISLAND_SIZE, ISLAND_SIZE, -ISLAND_SIZE, ISLAND_SIZE, -ISLAND_SIZE, ISLAND_SIZE);
	else
		ss->light_proj = glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 0.1f);
	// Render using scene graph.
	glDisable(GL_CULL_FACE);
	scene->pass(ss);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Greed::vr_render()
{
	std::vector<glm::mat4> eyeToHead, projectionMatrix;
	glm::mat4 headToBodyMatrix;
	eyeToHead.push_back(glm::mat4(1.0f));
	eyeToHead.push_back(glm::mat4(1.0f));
	projectionMatrix.push_back(glm::mat4(1.0f));
	projectionMatrix.push_back(glm::mat4(1.0f));
	vr::VRCompositor()->WaitGetPoses(GreedVR::vars.trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

	//Get Head and Eye Matrices
	const vr::HmdMatrix34_t headMatrix = GreedVR::vars.trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
	const vr::HmdMatrix34_t& ltMatrix = GreedVR::vars.hmd->GetEyeToHeadTransform(vr::Eye_Left);
	const vr::HmdMatrix34_t& rtMatrix = GreedVR::vars.hmd->GetEyeToHeadTransform(vr::Eye_Right);
	const vr::HmdMatrix44_t& ltProj = GreedVR::vars.hmd->GetProjectionMatrix(vr::Eye_Left, 0.01f, 1000.f, vr::API_OpenGL);
	const vr::HmdMatrix44_t& rtProj = GreedVR::vars.hmd->GetProjectionMatrix(vr::Eye_Right, 0.01f, 1000.f, vr::API_OpenGL);
	for (int r = 0; r < 3; ++r) {
		for (int c = 0; c < 4; ++c) {
			eyeToHead[0][c][r] = ltMatrix.m[r][c];
			eyeToHead[1][c][r] = rtMatrix.m[r][c];
			headToBodyMatrix[c][r] = headMatrix.m[r][c];
		}
	}
	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			projectionMatrix[0][c][r] = ltProj.m[r][c];
			projectionMatrix[1][c][r] = rtProj.m[r][c];
		}
	}

	/*Useless Code
	glm::mat4 bodyToWorldMatrix = glm::mat4(1.0f); //Useless
	glm::vec3 head_center = glm::vec3(0.f, 0.f, 0.f);

	//Get Body Matrix (for Room Space)	
	if (GreedVR::vars.trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		bodyToWorldMatrix = GreedVR::ConvertSteamVRMatrixToMatrix4(GreedVR::vars.trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);

		head_center = glm::vec3(bodyToWorldMatrix * glm::vec4(0, 0, 0, 1));

		//fprintf(stderr, "Printing for device %u: Center is %f\t%f\t%f\n", vr::k_unTrackedDeviceIndex_Hmd, center.x, center.y, center.z);
	}	
	*/

	for (int eye = 0; eye < GreedVR::vars.numEyes; ++eye) {
		glBindFramebuffer(GL_FRAMEBUFFER, GreedVR::vars.framebuffer[eye]);
		glViewport(0, 0, GreedVR::vars.framebufferWidth, GreedVR::vars.framebufferHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 proj = projectionMatrix[eye];
		glm::mat4 head = glm::inverse(eyeToHead[eye]) * glm::inverse(headToBodyMatrix);

		camera->cam_front = glm::mat3(glm::transpose(head)) * glm::vec3(0.f, 0.f, -1.f);
		camera->cam_front.y = 0.f;

		scene->P = proj;		
		camera->V = head * glm::inverse(glm::translate(glm::mat4(1.0f), camera->cam_pos));// *glm::inverse(glm::translate(glm::mat4(1.f), head_center * 10.f));
		scene->render();


		const vr::Texture_t tex = { reinterpret_cast<void*>(intptr_t(GreedVR::vars.colorRenderTarget[eye])), vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::EVREye(eye), &tex);
	}	

	//Process VR Event
	vr::VREvent_t event;
	while (GreedVR::vars.hmd->PollNextEvent(&event, sizeof(event)))
	{
		GreedVR::ProcessVREvent(event);
	}
}

void Greed::handle_movement()
{
	const GLfloat EDGE_LEEWAY = 100.f;
	const GLfloat MOVE_BOUNDS = ISLAND_SIZE + EDGE_LEEWAY;
	const GLfloat EDGE_THRESH = 20.f;
	const GLfloat BASE_CAM_SPEED = 2.f;
	GLfloat cam_step = keys[GLFW_KEY_LEFT_SHIFT] ? 5*BASE_CAM_SPEED : BASE_CAM_SPEED;
	glm::vec3 displacement(0.f);
	if (keys[GLFW_KEY_W])
		displacement += cam_step * camera->cam_front;
	if (keys[GLFW_KEY_S])
		displacement -= cam_step * camera->cam_front;
	if (keys[GLFW_KEY_A])
		displacement -= glm::normalize(glm::cross(camera->cam_front, camera->cam_up)) * cam_step;
	if (keys[GLFW_KEY_D])
		displacement += glm::normalize(glm::cross(camera->cam_front, camera->cam_up)) * cam_step;
	if (keys[GLFW_KEY_SPACE])
		displacement += cam_step * camera->cam_up;
	glm::vec3 new_pos = camera->cam_pos + displacement;

	// Check horizontal bounds.
	if (new_pos.x < -MOVE_BOUNDS || new_pos.x > MOVE_BOUNDS || new_pos.z < -MOVE_BOUNDS || new_pos.z > MOVE_BOUNDS)
		return;

	// Smoother edge movement.
	if (new_pos.x < -MOVE_BOUNDS + EDGE_THRESH) {
		float diff = glm::abs(-MOVE_BOUNDS - new_pos.x);
		displacement *= diff / EDGE_THRESH;
	}
	else if (new_pos.x > MOVE_BOUNDS - EDGE_THRESH) {
		float diff = glm::abs(MOVE_BOUNDS - new_pos.x);
		displacement *= diff / EDGE_THRESH;
	}
	if (new_pos.z < -MOVE_BOUNDS + EDGE_THRESH) {
		float diff = glm::abs(-MOVE_BOUNDS - new_pos.z);
		displacement *= diff / EDGE_THRESH;
	}
	else if (new_pos.z > MOVE_BOUNDS - EDGE_THRESH) {
		float diff = glm::abs(MOVE_BOUNDS - new_pos.z);
		displacement *= diff / EDGE_THRESH;
	}
	new_pos = camera->cam_pos + displacement;

	// Fix height.
	float new_height = 0.f;
	if (Util::within_rect(glm::vec2(new_pos.x, new_pos.z), glm::vec2(-ISLAND_SIZE, ISLAND_SIZE), glm::vec2(ISLAND_SIZE, -ISLAND_SIZE)))
		new_height = Terrain::height_lookup(new_pos.x, new_pos.z, ISLAND_SIZE * 2);
	new_pos.y = new_height + PLAYER_HEIGHT;

	camera->cam_pos = new_pos;
	camera->recalculate();
}

void Greed::handle_movement_vr()
{
	// check if somebody else has input focus
	if (GreedVR::vars.hmd->IsInputFocusCapturedByAnotherProcess())
		return;

	std::vector<float> vertdataarray;
	unsigned int controllerVertcount = 0;
	int trackedControllerCount = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		//Preliminary Checks
		if (!GreedVR::vars.hmd->IsTrackedDeviceConnected(unTrackedDevice))
			continue;
		if (GreedVR::vars.hmd->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;
		trackedControllerCount += 1;
		if (!GreedVR::vars.trackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		//Get Controller State
		vr::VRControllerState_t pControllerState;
		if (!GreedVR::vars.hmd->GetControllerState(unTrackedDevice, &pControllerState, sizeof(pControllerState)))
			continue;
		
		//Testing
		fprintf(stderr, "Controller Pressed: %llu\n", (unsigned long long) pControllerState.ulButtonPressed);
		fprintf(stderr, "Controller Touched: %llu\n", (unsigned long long) pControllerState.ulButtonTouched);				
		fprintf(stderr, "Controller Test 1: %f\n", (float)pControllerState.rAxis[TRACKPAD].x);
		fprintf(stderr, "Controller Test 1: %f\n", (float)pControllerState.rAxis[TRACKPAD].y);
		fprintf(stderr, "Controller Test 0: %f\n", (float)pControllerState.rAxis[TRIGGER].x);
		

		const GLfloat EDGE_THRESH = 20.f;
		const GLfloat BASE_CAM_SPEED = 2.f;
		GLfloat cam_step = keys[GLFW_KEY_LEFT_SHIFT] ? 5 * BASE_CAM_SPEED : BASE_CAM_SPEED;
		glm::vec3 displacement(0.f);		

		// Fix height to be based on heightmap.
		float last_height = Terrain::height_lookup(camera->cam_pos.x, camera->cam_pos.z, ISLAND_SIZE * 2);

		if (pControllerState.ulButtonPressed == TRACKPAD_ID)
		{
			//fprintf(stderr, "TRACKPAD PRESSED\n");
			if (pControllerState.rAxis[TRACKPAD].y > 0.0f)
			{
				displacement += cam_step * camera->cam_front;
			}
			else if (pControllerState.rAxis[TRACKPAD].y < 0.0f)
			{
				displacement -= cam_step * camera->cam_front;
			}
		}

		glm::vec3 new_pos = camera->cam_pos + displacement;		

		// Check horizontal bounds.
		if (new_pos.x < -ISLAND_SIZE || new_pos.x > ISLAND_SIZE || new_pos.z < -ISLAND_SIZE || new_pos.z > ISLAND_SIZE)
			return;

		// Smoother edge movement.
		if (new_pos.x < -ISLAND_SIZE + EDGE_THRESH)
		{
			float diff = glm::abs(-ISLAND_SIZE - new_pos.x);
			displacement *= diff / EDGE_THRESH;
		}
		else if (new_pos.x > ISLAND_SIZE - EDGE_THRESH)
		{
			float diff = glm::abs(ISLAND_SIZE - new_pos.x);
			displacement *= diff / EDGE_THRESH;
		}
		if (new_pos.z < -ISLAND_SIZE + EDGE_THRESH)
		{
			float diff = glm::abs(-ISLAND_SIZE - new_pos.z);
			displacement *= diff / EDGE_THRESH;
		}
		else if (new_pos.z > ISLAND_SIZE - EDGE_THRESH)
		{
			float diff = glm::abs(ISLAND_SIZE - new_pos.z);
			displacement *= diff / EDGE_THRESH;
		}
		new_pos = camera->cam_pos + displacement;

		// Recheck height.
		float new_height = Terrain::height_lookup(new_pos.x, new_pos.z, ISLAND_SIZE * 2);
		new_pos.y = new_height + PLAYER_HEIGHT;

		//Check if Slope is Climable

		//float MAX_SLOPE = 1.4f;
		float slope = (new_height - last_height) / (glm::distance(glm::vec2(camera->cam_pos.x, camera->cam_pos.z), glm::vec2(new_pos.x, new_pos.z)));
		fprintf(stderr, "Slope is: %f\n", slope);
		//if (slope < MAX_SLOPE)
		//{
			camera->cam_pos = new_pos;
		//}
			

	}	
}

void Greed::setup_callbacks()
{
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, resize_callback);
}

void Greed::setup_opengl()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		glfwTerminate();
	}
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Greed::error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

void Greed::resize_callback(GLFWwindow* window, int width, int height)
{
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
		scene->P = glm::perspective(FOV, (float)width / (float)height, 0.1f, FAR_PLANE);
}

void Greed::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		keys[key] = true;
		switch (key) {
			// Check if escape was pressed
		case GLFW_KEY_ESCAPE:
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_Q:
			debug_shadows = !debug_shadows;
			break;
		case GLFW_KEY_C:
			next_scene();
			break;
		case GLFW_KEY_Z:
			next_skybox();
			break;
		case GLFW_KEY_X:
			shadows_on = !shadows_on;
			break;
		case GLFW_KEY_F:
			generate_forest();
			break;
		case GLFW_KEY_M:
			generate_map();
			break;
		case GLFW_KEY_V:
			generate_village();
			break;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}
}

void Greed::cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos)
{
	glm::vec3 current_cursor_pos(x_pos, y_pos, 0);
	
	// First movement detected.
	if (!mouse_moved)
	{
		mouse_moved = true;
		last_cursor_pos = current_cursor_pos;
		return;
	}

	glm::vec3 cursor_delta = current_cursor_pos - last_cursor_pos;
	if (lmb_down && keys[GLFW_KEY_LEFT_CONTROL]) {
		int dir = cursor_delta.x > 0 ? 1 : -1;
		float rot_angle = dir * glm::length(cursor_delta) * 0.001f;
		scene->light_pos = glm::vec3(glm::rotate(glm::mat4(1.0f), rot_angle, glm::vec3(0.f, 0.f, 1.f)) * glm::vec4(scene->light_pos, 1.0f));
		
		/*
		float angle;
		// Horizontal rotation
		angle = (float)(cursor_delta.x) / 100.f;
		camera->cam_pos = glm::vec3(glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(camera->cam_pos, 1.f));
		camera->cam_up = glm::vec3(glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(camera->cam_up, 1.f));

		// Vertical rotation
		angle = (float)(-cursor_delta.y) / 100.f;
		glm::vec3 axis = glm::cross((camera->cam_pos - (camera->cam_pos + camera->cam_front)), camera->cam_up);
		camera->cam_pos = glm::vec3(glm::rotate(glm::mat4(1.f), angle, axis) * glm::vec4(camera->cam_pos, 1.f));
		camera->cam_up = glm::vec3(glm::rotate(glm::mat4(1.f), angle, axis) * glm::vec4(camera->cam_up, 1.f));
		camera->cam_front = glm::normalize(-camera->cam_pos);
		
		camera->recalculate();
		*/
	}
	else if (!keys[GLFW_KEY_LEFT_CONTROL]) {
		// Look around.
		GLfloat xoffset = cursor_delta.x;
		GLfloat yoffset = cursor_delta.y;
		GLfloat sensitivity = 0.5;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		camera->yaw += xoffset;
		camera->pitch += yoffset;

		if (camera->pitch > 89.0f)
			camera->pitch = 89.0f;
		if (camera->pitch < -89.0f)
			camera->pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
		front.y = -sin(glm::radians(camera->pitch));
		front.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
		camera->cam_front = glm::normalize(front);
		camera->recalculate();
	}

	last_cursor_pos = current_cursor_pos;
}

void Greed::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double x_pos, y_pos;
	glfwGetCursorPos(window, &x_pos, &y_pos);

	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		if (action == GLFW_PRESS) {
			lmb_down = true;
			last_cursor_pos = glm::vec3(x_pos, y_pos, 0.f);
		}
		else if (action == GLFW_RELEASE) {
			lmb_down = false;
		}
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (action == GLFW_PRESS) {
			rmb_down = true;
			last_cursor_pos = glm::vec3(x_pos, y_pos, 0.f);
		}
		else if (action == GLFW_RELEASE) {
			rmb_down = false;
		}
		break;
	default:
		break;
	}
}

void Greed::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	glm::vec3 trans_vec = (float)yoffset * glm::normalize(camera->cam_front);
	// Only y is relevant here, -1 is down, +1 is up
	camera->cam_pos = glm::vec3(glm::translate(glm::mat4(1.0f), trans_vec) * glm::vec4(camera->cam_pos, 1.0f));
	camera->recalculate();
}