#include "greed.h"
#include "window.h"
#include "skybox_shader.h"
#include "shadow_shader.h"
#include "geometry_generator.h"
#include "scene_model.h"
#include "scene_transform.h"

#include "util.h"
#include "colors.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* static var definitions */
Scene* Greed::scene;
SceneCamera* Greed::camera;
glm::vec3 Greed::last_cursor_pos;
vr_vars GreedVR::vars;
bool Greed::lmb_down = false;
bool Greed::rmb_down = false;
bool Greed::vr_on = false;
bool Greed::keys[1024];

Greed::Greed() {}

Greed::~Greed() {}

void Greed::destroy()
{
	// Free memory here.
	delete(scene);
	ShaderManager::destroy();
	GeometryGenerator::clean_up();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Greed::setup_shaders()
{
	// Load shaders via a shader manager.
	ShaderManager::create_shader_program("basic");
	ShaderManager::create_shader_program("skybox");
	ShaderManager::create_shader_program("shadow");
	ShaderManager::set_default("basic");
}

void Greed::setup_scene()
{
	scene = new Scene();
	camera = scene->camera;
	SceneGroup *root = scene->root;

	// Do skybox.
	Material default_material;
	Mesh skybox_mesh = { nullptr, default_material, ShaderManager::get_shader_program("skybox") };
	SceneModel *skybox_model = new SceneModel(scene);
	skybox_model->add_mesh(skybox_mesh);
	root->add_child(skybox_model);

	// Generate geometry and create Mesh + SceneModel.
	Geometry *cube_geometry = GeometryGenerator::generate_cube(1.f, true);
	Material cube_material;
	cube_material.diffuse = cube_material.ambient = color::red;
	Mesh cube_mesh = { cube_geometry, cube_material, ShaderManager::get_default() };
	SceneModel *cube_model = new SceneModel(scene);
	cube_model->add_mesh(cube_mesh);
	SceneTransform *cube_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.f, 0.0f)));
	//SceneTransform *cube_translate = new SceneTransform(scene, glm::rotate(glm::mat4(1.f), glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f)));

	cube_translate->add_child(cube_model);
	root->add_child(cube_translate);

	// Water Plane
	Geometry *plane_geo = GeometryGenerator::generate_plane(1.f);
	Material water_material;
	water_material.diffuse = water_material.ambient = color::ocean_blue;
	Mesh water_mesh = { plane_geo, water_material, ShaderManager::get_default() };
	SceneModel *water_model = new SceneModel(scene);
	water_model->add_mesh(water_mesh);
	SceneTransform *water_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(200.0f, 1.0f, 200.0f)));
	SceneTransform *water_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.2f, 0.0f)));
	water_scale->add_child(water_model);
	water_translate->add_child(water_scale);
	root->add_child(water_translate);

	// Beach Plane (Temporary)
	Geometry *bez_plane_geo = GeometryGenerator::generate_bezier_plane(10.f, 50, 150, 0.1f, 0);
	Material beach_material;
	beach_material.diffuse = beach_material.ambient = color::windwaker_sand;
	Mesh beach_mesh = { bez_plane_geo, beach_material, ShaderManager::get_default() };
	SceneModel *beach_model = new SceneModel(scene);
	beach_model->add_mesh(beach_mesh);
	SceneTransform *beach_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(10.0f, 1.0f, 10.0f)));
	SceneTransform *beach_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.1f, 0.0f)));
	beach_scale->add_child(beach_model);
	beach_translate->add_child(beach_scale);
	root->add_child(beach_translate);

	// Island Land Grid
	Geometry *grid_geo = GeometryGenerator::generate_grid(9, 200.f, 30, 100.f, 777);
	Material land_material;
	land_material.diffuse = land_material.ambient = color::windwaker_green;
	Mesh land_mesh = { grid_geo, land_material, ShaderManager::get_default() };
	SceneModel *land_model = new SceneModel(scene);
	land_model->add_mesh(land_mesh);
	SceneTransform *land_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(0.25f, 0.25f, 0.25f)));
	SceneTransform *land_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.0f, 0.0f)));
	land_scale->add_child(land_model);
	land_translate->add_child(land_scale);
	root->add_child(land_translate);
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

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		handle_movement();

		// First pass: shadowmap.
		shadow_pass();

		// Second pass: usual rendering.
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (vr_on)
		{
			vr_render();
		}
		else {
			scene->render();
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
	// Render using scene graph.
	scene->pass(ss);
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
	const GLfloat cam_step = 1.01f;//0.01f; <-- too slow on lab computers
	if (keys[GLFW_KEY_W])
		camera->cam_pos += cam_step * camera->cam_front;
	if (keys[GLFW_KEY_S])
		camera->cam_pos -= cam_step * camera->cam_front;
	if (keys[GLFW_KEY_A])
		camera->cam_pos -= glm::normalize(glm::cross(camera->cam_front, camera->cam_up)) * cam_step;
	if (keys[GLFW_KEY_D])
		camera->cam_pos += glm::normalize(glm::cross(camera->cam_front, camera->cam_up)) * cam_step;
	if (keys[GLFW_KEY_SPACE])
		camera->cam_pos += cam_step * camera->cam_up;
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
		scene->P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
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
	glm::vec3 cursor_delta = current_cursor_pos - last_cursor_pos;
	if (lmb_down) {
		glm::vec3 rot_axis = glm::cross(last_cursor_pos, current_cursor_pos);
		float rot_angle = glm::length(cursor_delta) * 0.001f;
		scene->light_pos = glm::vec3(glm::rotate(glm::mat4(1.0f), rot_angle, rot_axis) * glm::vec4(scene->light_pos, 1.0f));
		
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
	else {
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