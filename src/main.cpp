#include "main.h"
#include "window.h"
#include "shader_manager.h"
#include "skybox_shader.h"
#include "geometry_generator.h"
#include "scene_model.h"
#include "scene_transform.h"
#include "scene.h"
#include "util.h"

GLFWwindow *window;
ShaderManager *shader_manager;
Scene *scene;
bool lmb_down;
bool rmb_down;
glm::vec3 last_cursor_pos;

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

void resize_callback(GLFWwindow* window, int width, int height)
{
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
		scene->P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
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

void cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos)
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	glm::vec3 trans_vec = (float)yoffset * glm::normalize(scene->camera->cam_look_at - scene->camera->cam_pos);
	// Only y is relevant here, -1 is down, +1 is up
	scene->camera->cam_pos = glm::vec3(glm::translate(glm::mat4(1.0f), trans_vec) * glm::vec4(scene->camera->cam_pos, 1.0f));
	scene->camera->recalculate_camera();
}

void setup_callbacks()
{
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, resize_callback);
}

void setup_glew()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		glfwTerminate();
	}
}

void setup_opengl_settings()
{
	setup_glew();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void destroy()
{
	// Free memory here.
	delete(shader_manager);
	delete(scene);
	GeometryGenerator::clean_up();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void setup_shaders()
{
	// Load shaders via a shader manager.
	shader_manager->create_shader_program("basic");
	shader_manager->create_shader_program("skybox");
	shader_manager->set_default("basic");
}

void setup_scene()
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
	cube_material.diffuse = { 1.f, 0.f, 0.f };
	cube_material.ambient = { 1.f, 0.f, 0.f };
	Mesh cube_mesh = { cube_geometry, cube_material, shader_manager->get_default() };
	SceneModel *cube_model = new SceneModel(scene);
	cube_model->add_mesh(cube_mesh);
	root->add_child(cube_model);
}

int main()
{
	window = Window::create_window(1280, 720, "Greed Island");
	shader_manager = new ShaderManager();
	setup_callbacks();
	setup_opengl_settings();
	setup_shaders();
	setup_scene();
	// Setup VR.

	// Send height/width of window
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	resize_callback(window, width, height);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene->render();
		glfwSwapBuffers(window);
	}

	destroy();
	exit(EXIT_SUCCESS);
}