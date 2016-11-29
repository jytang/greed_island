#include "greed.h"
#include "window.h"
#include "skybox_shader.h"
#include "geometry_generator.h"
#include "scene_model.h"
#include "scene_transform.h"

#include "util.h"
#include "colors.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* static var declarations */
Scene* Greed::scene;
bool Greed::lmb_down;
bool Greed::rmb_down;
glm::vec3 Greed::last_cursor_pos;

Greed::Greed() {}

Greed::~Greed() {}

void Greed::destroy()
{
	// Free memory here.
	delete(shader_manager);
	delete(scene);
	GeometryGenerator::clean_up();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Greed::setup_shaders()
{
	// Load shaders via a shader manager.
	shader_manager->create_shader_program("basic");
	shader_manager->create_shader_program("skybox");
	shader_manager->set_default("basic");
}

void Greed::setup_scene()
{
	scene = new Scene();
	SceneGroup *root = scene->root;

	// Do skybox.
	Material default_material;
	Mesh skybox_mesh = { nullptr, default_material, shader_manager->get_shader_program("skybox") };
	SceneModel *skybox_model = new SceneModel(scene);
	skybox_model->add_mesh(skybox_mesh);
	root->add_child(skybox_model);

	// Generate geometry and create Mesh + SceneModel.
	Geometry *cube_geometry = GeometryGenerator::generate_cube(1.f, true);
	Material cube_material;
	cube_material.diffuse = cube_material.ambient = color::red;
	Mesh cube_mesh = { cube_geometry, cube_material, shader_manager->get_default() };
	SceneModel *cube_model = new SceneModel(scene);
	cube_model->add_mesh(cube_mesh);
	root->add_child(cube_model);

	// Water Plane
	Geometry *plane_geo = GeometryGenerator::generate_plane(1.f);
	Material water_material;
	water_material.diffuse = water_material.ambient = color::ocean_blue;
	Mesh water_mesh = { plane_geo, water_material, shader_manager->get_default() };
	SceneModel *water_model = new SceneModel(scene);
	water_model->add_mesh(water_mesh);
	SceneTransform *water_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(1000.0f, 1.0f, 1000.0f)));
	SceneTransform *water_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.02f, 0.0f)));
	water_scale->add_child(water_model);
	water_translate->add_child(water_scale);
	root->add_child(water_translate);

	// Beach Plane (Temporary)
	Material beach_material;
	beach_material.diffuse = beach_material.ambient = color::windwaker_sand;
	Mesh beach_mesh = { plane_geo, beach_material, shader_manager->get_default() };
	SceneModel *beach_model = new SceneModel(scene);
	beach_model->add_mesh(beach_mesh);
	SceneTransform *beach_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(10.0f, 1.0f, 10.0f)));
	SceneTransform *beach_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.01f, 0.0f)));
	beach_scale->add_child(beach_model);
	beach_translate->add_child(beach_scale);
	root->add_child(beach_translate);

	// Island Plane (Temporary)
	Material land_material;
	land_material.diffuse = land_material.ambient = color::windwaker_green;
	Mesh land_mesh = { plane_geo, land_material, shader_manager->get_default() };
	SceneModel *land_model = new SceneModel(scene);
	land_model->add_mesh(land_mesh);
	SceneTransform *land_scale = new SceneTransform(scene, glm::scale(glm::mat4(1.f), glm::vec3(8.0f, 1.0f, 8.0f)));
	SceneTransform *land_translate = new SceneTransform(scene, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.0f, 0.0f)));
	land_scale->add_child(land_model);
	land_translate->add_child(land_scale);
	root->add_child(land_translate);
}

void Greed::go()
{
	window = Window::create_window(1280, 720, "Greed Island");
	shader_manager = new ShaderManager();
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
		scene->camera->V = head;
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
		switch (key) {
			// Check if escape was pressed
		case GLFW_KEY_ESCAPE:
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		default:
			break;
		}
	}
}

void Greed::cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos)
{
	glm::vec3 current_cursor_pos(x_pos, y_pos, 0);
	glm::vec3 cursor_delta = current_cursor_pos - last_cursor_pos;
	if (lmb_down) {
		float angle;

		// Horizontal rotation
		angle = (float)(cursor_delta.x) / 100.f;
		scene->camera->cam_pos = glm::vec3(glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(scene->camera->cam_pos, 1.f));
		scene->camera->cam_up = glm::vec3(glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(scene->camera->cam_up, 1.f));

		// Vertical rotation
		angle = (float)(-cursor_delta.y) / 100.f;
		glm::vec3 axis = glm::cross((scene->camera->cam_pos - scene->camera->cam_look_at), scene->camera->cam_up);
		scene->camera->cam_pos = glm::vec3(glm::rotate(glm::mat4(1.f), angle, axis) * glm::vec4(scene->camera->cam_pos, 1.f));
		scene->camera->cam_up = glm::vec3(glm::rotate(glm::mat4(1.f), angle, axis) * glm::vec4(scene->camera->cam_up, 1.f));

		scene->camera->recalculate_camera();
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
	glm::vec3 trans_vec = (float)yoffset * glm::normalize(scene->camera->cam_look_at - scene->camera->cam_pos);
	// Only y is relevant here, -1 is down, +1 is up
	scene->camera->cam_pos = glm::vec3(glm::translate(glm::mat4(1.0f), trans_vec) * glm::vec4(scene->camera->cam_pos, 1.0f));
	scene->camera->recalculate_camera();
}