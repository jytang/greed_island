#include "greed.h"
#include "window.h"
#include "skybox_shader.h"
#include "shadow_shader.h"
#include "geometry_generator.h"
#include "scene_model.h"
#include "scene_transform.h"
#include "scene_animation.h"
#include "island_scene.h"
#include "desert_scene.h"
#include "snow_scene.h"
#include "space_scene.h"
#include "fire_scene.h"
#include <cfloat>

#include "util.h"
#include "colors.h"
#include "global.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool vr_on = false;

/* global vars */
vr_vars GreedVR::vars;
Scene* scene;
IslandScene* island_scene;
DesertScene* desert_scene;
SnowScene* snow_scene;
SpaceScene* space_scene;
FireScene* fire_scene;
std::vector<Scene*> scenes;
SceneCamera* camera;

bool keys[1024];
bool lmb_down = false;
bool rmb_down = false;
bool debug_shadows = false;
bool mouse_moved = false;
bool shadows_on = true;
bool god_mode = false;
bool helicopter_mode = false;
glm::vec3 last_cursor_pos;

const GLfloat PLAYER_HEIGHT = Global::PLAYER_HEIGHT;

const GLfloat FAR_PLANE = 50.f * PLAYER_HEIGHT;
const GLfloat FOV = 45.f;

const GLfloat   BASE_CAM_SPEED = PLAYER_HEIGHT / 10.f;
const GLfloat   EDGE_PAN_THRESH = 5.f;
const GLfloat   EDGE_PAN_SPEED = 0.5f;

SceneTransform *controller_1_translate;
SceneTransform *controller_2_translate;

Greed::Greed() {}

Greed::~Greed() {}

void Greed::destroy()
{
	// Free memory here.
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

void Greed::setup_scenes()
{
	island_scene = new IslandScene();
	desert_scene = new DesertScene();
	snow_scene = new SnowScene();
	space_scene = new SpaceScene();
	fire_scene = new FireScene();
	scenes.push_back(island_scene);
	scenes.push_back(desert_scene);
	scenes.push_back(snow_scene);
	scenes.push_back(space_scene);
	scenes.push_back(fire_scene);

	// Initial scene and cam.
	change_scene(island_scene);

	// Skybox
	Material default_material;
	Mesh skybox_mesh = { nullptr, default_material, ShaderManager::get_shader_program("skybox"), glm::mat4(1.f) };
	SceneModel *skybox_model = new SceneModel(scene);
	skybox_model->add_mesh(skybox_mesh);

	for (Scene* s : scenes)
	{
		s->camera = camera; // Set all cameras to be the same.
		s->root->add_child(skybox_model); // Skyboxes for all scenes.
		s->setup();
	}

	//Show Vive Controllers using Two Blue Spheres
	if (vr_on)
	{
		Geometry *sphere_geo = GeometryGenerator::generate_sphere(1.f, 7);
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
		scene->root->add_child(controller_1_translate);
		scene->root->add_child(controller_2_translate);
	}
}

void Greed::go()
{
	window = Window::create_window(0, 0, "Greed Island");
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // Don't show cursor
	setup_callbacks();
	setup_opengl();
	if (vr_on) GreedVR::init();

	setup_shaders();
	// Seed PRNG.
	Util::seed(0);
	setup_scenes();

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
			//std::cerr << "FPS: " << frame << std::endl;
			frame = 0;
			prev_ticks = curr_time;
		}
		if (curr_time - move_prev_ticks > 1.f / 60.f)
		{
			if (helicopter_mode && scene == island_scene)
				((IslandScene *)scene)->handle_helicopter();
			else
			{
				if (vr_on)
					handle_movement_vr();
				else
					handle_movement();
			}

			// Rotate sun.
			if (keys[GLFW_KEY_LEFT_ALT])
			{
				float sun_step = 0.005f * PLAYER_HEIGHT;

				if (scene->light_pos.y < 0.5f * PLAYER_HEIGHT)
					sun_step *= 2;
				else
					sun_step *= glm::max(scene->light_pos.y / (scene->get_size() * 2), 0.1f * PLAYER_HEIGHT);

				sun_step *= scene->light_pos.y < 0 ? 3 : 1;
				scene->light_pos = glm::vec3(glm::rotate(glm::mat4(1.0f), sun_step, glm::vec3(0.f, 0.f, 1.f)) * glm::vec4(scene->light_pos, 1.0f));
			}

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

	//Useless
	glm::mat4 bodyToWorldMatrix = glm::mat4(1.0f);
	glm::vec3 head_center = glm::vec3(0.f, 0.f, 0.f);

	//Get Body Matrix (for Room Space)	
	if (GreedVR::vars.trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		bodyToWorldMatrix = GreedVR::ConvertSteamVRMatrixToMatrix4(GreedVR::vars.trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);

		head_center = glm::vec3(bodyToWorldMatrix * glm::vec4(0, 0, 0, 1));

		//fprintf(stderr, "Printing for device %u: Center is %f\t%f\t%f\n", vr::k_unTrackedDeviceIndex_Hmd, center.x, center.y, center.z);
	}	
	


	for (int eye = 0; eye < GreedVR::vars.numEyes; ++eye) {
		glBindFramebuffer(GL_FRAMEBUFFER, GreedVR::vars.framebuffer[eye]);
		glViewport(0, 0, GreedVR::vars.framebufferWidth, GreedVR::vars.framebufferHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 proj = projectionMatrix[eye];
		glm::mat4 head = glm::inverse(eyeToHead[eye]) * glm::inverse(headToBodyMatrix);

		camera->cam_front = glm::mat3(glm::transpose(head)) * glm::vec3(0.f, 0.f, -1.f);
		camera->cam_front.y = 0.f;

		scene->P = proj;		
		camera->V = head * glm::inverse(glm::translate(glm::mat4(1.0f), camera->cam_pos));
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
	GLfloat cam_step = keys[GLFW_KEY_LEFT_SHIFT] ? 3*BASE_CAM_SPEED : BASE_CAM_SPEED;
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

	// Don't limit movement if godmode is on.
	if (god_mode)
	{
		camera->cam_pos += displacement;
		camera->recalculate();
	}
	else {
		scene->displace_cam(displacement);
	}
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
		/*
		fprintf(stderr, "Controller Pressed: %llu\n", (unsigned long long) pControllerState.ulButtonPressed);
		fprintf(stderr, "Controller Touched: %llu\n", (unsigned long long) pControllerState.ulButtonTouched);
		fprintf(stderr, "Controller Test 1: %f\n", (float)pControllerState.rAxis[TRACKPAD].x);
		fprintf(stderr, "Controller Test 1: %f\n", (float)pControllerState.rAxis[TRACKPAD].y);
		fprintf(stderr, "Controller Test 0: %f\n", (float)pControllerState.rAxis[TRIGGER].x);
		*/

		GLfloat cam_step = keys[GLFW_KEY_LEFT_SHIFT] ? 3 * BASE_CAM_SPEED : BASE_CAM_SPEED;
		glm::vec3 displacement(0.f);

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

		scene->displace_cam(displacement);
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
	{
		for (Scene * s : scenes)
			s->P = glm::perspective(FOV, (float)width / (float)height, 0.1f, FAR_PLANE);
	}
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
		case GLFW_KEY_X:
			shadows_on = !shadows_on;
			break;
		case GLFW_KEY_F:
			if (scene == island_scene)
			{
				if (keys[GLFW_KEY_LEFT_SHIFT])
					((IslandScene *) scene)->generate_small_forest();
				else
					((IslandScene *)scene)->generate_forest();
			}
			break;
		case GLFW_KEY_M:
			if (scene == island_scene)
			{
				((IslandScene *)scene)->generate_map();
				((IslandScene *)scene)->generate_small_map();
			}
			break;
		case GLFW_KEY_V:
			if (scene == island_scene)
			{
				if (keys[GLFW_KEY_LEFT_SHIFT])
					((IslandScene *)scene)->generate_small_village();
				else
					((IslandScene *)scene)->generate_village();
			}
			break;
		case GLFW_KEY_G:
			god_mode = !god_mode;
			break;
		case GLFW_KEY_H:
			if (scene == island_scene)
			{
				helicopter_mode = !helicopter_mode;
				((IslandScene *)scene)->helicopter_angle = 0;
			}
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
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
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

		if (current_cursor_pos.x > width - EDGE_PAN_THRESH)
			xoffset = EDGE_PAN_SPEED;
		else if (current_cursor_pos.x < EDGE_PAN_THRESH)
			xoffset = -EDGE_PAN_SPEED;

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