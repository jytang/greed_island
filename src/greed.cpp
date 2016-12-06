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
bool keys[1024];
bool lmb_down = false;
bool rmb_down = false;
bool debug_shadows = false;
bool vr_on = false;
bool mouse_moved = false;
const GLfloat FAR_PLANE = 1000.f;
glm::vec3 last_cursor_pos;

const GLfloat ISLAND_SIZE = 600.f;
const GLfloat PLAYER_HEIGHT = 20.f;

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

void Greed::setup_scene()
{
	island_scene = new Scene();
	transition_scene = new Scene();
	desert_scene = new Scene();
	scenes.push_back(island_scene);
	scenes.push_back(transition_scene);
	scenes.push_back(desert_scene);

	scene = island_scene;
	camera = scene->camera;
	scene->light_pos = glm::vec3(0.f, ISLAND_SIZE*2, ISLAND_SIZE);
	SceneGroup *root = scene->root;
	
	// Set all cameras to be the same.
	for (Scene* s : scenes)
	{
		s->camera = camera;
	}

	// Seed PRNG.
	Util::seed(0);

	// Do skybox.
	Material default_material;
	Mesh skybox_mesh = { nullptr, default_material, ShaderManager::get_shader_program("skybox"), glm::mat4(1.f) };
	SceneModel *skybox_model = new SceneModel(scene);
	skybox_model->add_mesh(skybox_mesh);
	root->add_child(skybox_model);
	transition_scene->root->add_child(skybox_model);

	Geometry *cylinder_geo = GeometryGenerator::generate_cylinder(0.25f, 2.f, 3, false);
	Geometry *diamond_geo = GeometryGenerator::generate_sphere(2.f, 3);
	Geometry *sphere_geo = GeometryGenerator::generate_sphere(1.f, 7);
	Geometry *cube_geometry = GeometryGenerator::generate_cube(1.f, true);

	Material sphere_material;
	sphere_material.diffuse = sphere_material.ambient = color::ocean_blue;
	Mesh sphere_mesh = { diamond_geo, sphere_material, ShaderManager::get_default(), glm::mat4(1.f) };
	SceneModel *sphere_model = new SceneModel(scene);
	sphere_model->add_mesh(sphere_mesh);
	SceneTransform *sphere_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(1.f, 1.0f, 1.f)));
	SceneTransform *sphere_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 100.f, 100.0f)));
	sphere_scale->add_child(sphere_model);
	sphere_translate->add_child(sphere_scale);
	//root->add_child(sphere_translate);

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
	const GLuint    NUM_TREES = 10;
	const GLfloat   PERCENT_TREE_ANIM = 0.9f;
	const GLuint    NUM_TREE_TYPES = 10;
	const GLfloat   TREE_SCALE = 1.5f;
	const GLfloat   PATH_WIDTH = 90.f;
	const GLfloat   FOREST_RADIUS = ISLAND_SIZE / 1.1f;
	const GLint		NUM_BUILDINGS = 5;
	const GLfloat	VILLAGE_DIAMETER_TRUE = ((float)VILLAGE_DIAMETER / HEIGHT_MAP_SIZE) * TERRAIN_SIZE * TERRAIN_SCALE;
	const GLfloat   WATER_SCALE = ISLAND_SIZE * 4;
	const GLint     CAM_OFFSET = 20;

	// Water Plane
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

	// New Terrain Method using Awesomeness
	std::cerr << "Generating Height Map" << std::endl;
	Terrain::generate_height_map(HEIGHT_MAP_SIZE, HEIGHT_MAP_MAX, VILLAGE_DIAMETER, HEIGHT_RANDOMNESS_SCALE, 0);

	float cam_height = Terrain::height_lookup(0.f, ISLAND_SIZE - CAM_OFFSET, ISLAND_SIZE * 2);
	camera->cam_pos = glm::vec3(0.f, cam_height+PLAYER_HEIGHT, ISLAND_SIZE - CAM_OFFSET);
	camera->recalculate();

	std::cerr << "Generating Land Terrain" << std::endl;
	// Second Parameter Below is Resolution^2 of Island, LOWER TO RUN FASTER
	Geometry *land_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, BEACH_HEIGHT, HEIGHT_MAP_MAX * 0.8f - 0.2f);
	Material land_material;
	land_material.diffuse = land_material.ambient = color::windwaker_green;
	Mesh land_mesh = { land_geo, land_material, ShaderManager::get_default(), glm::mat4(1.f) };	

	std::cerr << "Generating Plateau Terrain" << std::endl;
	Geometry *plateau_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, HEIGHT_MAP_MAX * 0.8f - 0.2f, HEIGHT_MAP_MAX);
	Material plateau_material;
	plateau_material.diffuse = plateau_material.ambient = color::bone_white;
	Mesh plateau_mesh = { plateau_geo, plateau_material, ShaderManager::get_default(), glm::mat4(1.f) };

	std::cerr << "Generating Sand Terrain" << std::endl;
	Geometry *sand_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, 0.0f, BEACH_HEIGHT);
	Material sand_material;
	sand_material.diffuse = sand_material.ambient = color::windwaker_sand;
	Mesh sand_mesh = { sand_geo, sand_material, ShaderManager::get_default(), glm::mat4(1.f) };

	SceneModel *terrain_model = new SceneModel(scene);
	terrain_model->add_mesh(land_mesh);
	terrain_model->add_mesh(sand_mesh);
	terrain_model->add_mesh(plateau_mesh);
	SceneTransform *terrain_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(TERRAIN_SCALE, 1.f, TERRAIN_SCALE)));
	terrain_scale->add_child(terrain_model);
	root->add_child(terrain_scale);

	// Beach Plane
	Geometry *beach_geo = GeometryGenerator::generate_bezier_plane(ISLAND_SIZE*1.5f, 50, 150, 0.1f, 0);
	Material beach_material = sand_material;
	Mesh beach_mesh = { beach_geo, beach_material, ShaderManager::get_default(), glm::mat4(1.f) };
	SceneModel *beach_model = new SceneModel(scene);
	beach_model->add_mesh(beach_mesh);
	SceneTransform *beach_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(1.0f, 1.0f, 1.0f)));
	SceneTransform *beach_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.1f, 0.0f)));
	beach_scale->add_child(beach_model);
	beach_translate->add_child(beach_scale);
	root->add_child(beach_translate);
	desert_scene->root->add_child(beach_translate);

	std::cerr << "Generating Forest" << std::endl;
	glm::vec3 leaf_colors[] = {color::olive_green, color::olive_green, color::olive_green, color::autumn_orange, color::purple, color::bone_white, color::indian_red};
	glm::vec3 branch_colors[] = { color::brown, color::wood_saddle, color::wood_sienna, color::wood_tan, color::wood_tan_light };
	Material branch_material, leaf_material;
	SceneGroup *forest = new SceneGroup(scene);
	for (int i = 0; i < NUM_TREES; ++i) {
		//SceneGroup *tree = tree_types[i % tree_types.size()];
		leaf_material.diffuse = leaf_material.ambient = leaf_colors[(int)Util::random(0,7)];
		branch_material.diffuse = branch_material.ambient = branch_colors[(int)Util::random(0, 5)];
		bool animated = false;
		if (i % (NUM_TREES / (int) (NUM_TREES*PERCENT_TREE_ANIM)) == 0)
			animated = true;
		if (i % 50 == 0)
			std::cerr << "Tree " << i << std::endl;
		float x, z;
		do {
			float angle = Util::random(0, 360);
			float distance = Util::random(VILLAGE_DIAMETER*TERRAIN_SCALE / 3, FOREST_RADIUS);

			x = glm::cos(glm::radians(angle)) * distance;
			z = glm::sin(glm::radians(angle)) * distance;
		} while (Util::within_rect(glm::vec2(x, z), glm::vec2(-PATH_WIDTH/2, FOREST_RADIUS), glm::vec2(PATH_WIDTH/2, 0)));
		float y = Terrain::height_lookup(x, z, ISLAND_SIZE*2);
		glm::vec3 location = {x, y, z};

		SceneGroup *tree = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 7, 1, 20.f, 2.f, branch_material, leaf_material, animated, location, 0);
		
		// THIS IS NASTY CODE:
		((SceneModel *)(tree->children[0]))->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		if (animated)
			((SceneModel *)((SceneAnimation *)(tree->children[1]))->children[0])->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		else
			((SceneModel *)(tree->children[1]))->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		// END NASTY CODE

		forest->add_child(tree);

		//SceneTransform *tree_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), location));
		//tree_translate->add_child(tree);
		//root->add_child(tree_translate);
	}
	root->add_child(forest);
	std::cerr << "Done." << std::endl;

	// Village
	SceneGroup *village = new SceneGroup(scene);
	for (int i = 0; i < NUM_BUILDINGS; ++i)
	{

		std::cerr << "House " << i << std::endl;

		float x, z;

		float angle = ((350.f / (NUM_BUILDINGS)) * i) + 130.f; //Circles around, starting from the far side
		float distance = VILLAGE_DIAMETER_TRUE / 3.f;

		x = glm::cos(glm::radians(angle)) * distance;
		z = glm::sin(glm::radians(angle)) * distance;

		float y = Terrain::height_lookup(x, z, ISLAND_SIZE * 2);
		glm::vec3 location = { x, y, z };

		SceneModel *building = ShapeGrammar::generate_building(scene, 0);

		SceneTransform *building_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), location));
		building_translate->add_child(building);
		village->add_child(building_translate);
	}
	root->add_child(village);
	std::cerr << "Done." << std::endl;

	// Small-scale showcase.
	const GLfloat SMALL_ROT_SPEED = 0.3f;
	SceneTransform *small_terrain_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.05f/10.f, 0.05f)));
	SceneAnimation *small_terrain_anim = new SceneAnimation(scene, 0.f, FLT_MAX, 0.f, SMALL_ROT_SPEED, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
	SceneTransform *small_terrain_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(50.f, 175.f, -50.f)));
	small_terrain_scale->add_child(terrain_model);
	small_terrain_anim->add_child(small_terrain_scale);
	small_terrain_translate->add_child(small_terrain_anim);
	root->add_child(small_terrain_translate);

	SceneGroup *small_tree_1 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 2, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_2 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 3, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_3 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 4, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_4 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 5, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_5 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 6, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_tree_6 = Tree::generate_tree(scene, cylinder_geo, diamond_geo, 7, 1, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), 777);
	SceneGroup *small_trees[6] = { small_tree_1, small_tree_2, small_tree_3, small_tree_4, small_tree_5, small_tree_6 };
	for (int i = 0; i < 6; ++i)
	{
		SceneTransform *small_tree_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(0.05f)));
		SceneAnimation *small_tree_anim = new SceneAnimation(scene, 0.f, FLT_MAX, 0.f, SMALL_ROT_SPEED, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		SceneTransform *small_tree_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(-50.f + (i * 5.f), 175.f, -50.f)));
		small_tree_scale->add_child(small_trees[i]);
		small_tree_anim->add_child(small_tree_scale);
		small_tree_translate->add_child(small_tree_anim);
		root->add_child(small_tree_translate);
	}
}

void Greed::go()
{
	window = Window::create_window(1280, 720, "Greed Island");
	setup_callbacks();
	setup_opengl();
	if (vr_on) GreedVR::init();

	setup_shaders();
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
			handle_movement();
			move_prev_ticks = curr_time;
		}

		glfwGetFramebufferSize(window, &width, &height);
		scene->update_frustum_planes();
		scene->update_frustum_corners(width, height, FAR_PLANE);

		// First pass: shadowmap.
		shadow_pass();

		// Second pass: usual rendering.
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (vr_on)
		{
			vr_render();
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
	ss->light_proj = scene->frustum_ortho();//glm::ortho(-ISLAND_SIZE, ISLAND_SIZE, -ISLAND_SIZE, ISLAND_SIZE, -ISLAND_SIZE, ISLAND_SIZE);
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
	for (int eye = 0; eye < GreedVR::vars.numEyes; ++eye) {
		glBindFramebuffer(GL_FRAMEBUFFER, GreedVR::vars.framebuffer[eye]);
		glViewport(0, 0, GreedVR::vars.framebufferWidth, GreedVR::vars.framebufferHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 proj = projectionMatrix[eye];
		glm::mat4 head = glm::inverse(eyeToHead[eye]) * glm::inverse(headToBodyMatrix);

		scene->P = proj;
		camera->V = head;
		scene->render();

		//RenderControllerAxes(trackedDevicePose);

		const vr::Texture_t tex = { reinterpret_cast<void*>(intptr_t(GreedVR::vars.colorRenderTarget[eye])), vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::EVREye(eye), &tex);
	}

	// Mirror to the window
	/*glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
	glViewport(0, 0, Window::width, Window::height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlitFramebuffer(0, 0, framebufferWidth, framebufferHeight, 0, 0, Window::width, Window::height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);*/

	//Process VR Event
	vr::VREvent_t event;
	while (GreedVR::vars.hmd->PollNextEvent(&event, sizeof(event)))
	{
		GreedVR::ProcessVREvent(event);
	}
}

void Greed::handle_movement()
{
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

	// Fix height to be based on heightmap.
	float new_height = Terrain::height_lookup(new_pos.x, new_pos.z, ISLAND_SIZE * 2);
	if (new_pos.y - PLAYER_HEIGHT < new_height)
		new_pos.y = new_height + PLAYER_HEIGHT;

	// Check horizontal bounds.
	if (new_pos.x < -ISLAND_SIZE || new_pos.x > ISLAND_SIZE || new_pos.z < -ISLAND_SIZE || new_pos.z > ISLAND_SIZE)
		return;

	// Smoother edge movement.
	if (new_pos.x < -ISLAND_SIZE + EDGE_THRESH) {
		float diff = glm::abs(-ISLAND_SIZE - new_pos.x);
		displacement *= diff / EDGE_THRESH;
	}
	else if (new_pos.x > ISLAND_SIZE - EDGE_THRESH) {
		float diff = glm::abs(ISLAND_SIZE - new_pos.x);
		displacement *= diff / EDGE_THRESH;
	}
	if (new_pos.z < -ISLAND_SIZE + EDGE_THRESH) {
		float diff = glm::abs(-ISLAND_SIZE - new_pos.z);
		displacement *= diff / EDGE_THRESH;
	}
	else if (new_pos.z > ISLAND_SIZE - EDGE_THRESH) {
		float diff = glm::abs(ISLAND_SIZE - new_pos.z);
		displacement *= diff / EDGE_THRESH;
	}
	new_pos = camera->cam_pos + displacement;

	// Recheck height.
	new_height = Terrain::height_lookup(new_pos.x, new_pos.z, ISLAND_SIZE * 2);
	if (new_pos.y - PLAYER_HEIGHT < new_height)
		new_pos.y = new_height + PLAYER_HEIGHT;

	camera->cam_pos = new_pos;
	camera->recalculate();
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
		scene->P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, FAR_PLANE);
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
		case GLFW_KEY_V:
			//vr_on = !vr_on;
			break;
		case GLFW_KEY_R:
			camera->reset();
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