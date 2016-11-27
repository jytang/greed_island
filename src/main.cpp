#include "main.h"
#include "window.h"
#include "input_manager.h"
#include "shader_manager.h"
#include "geometry_generator.h"
#include "scene_model.h"
#include "scene_transform.h"
#include "scene.h"
#include "util.h"

GLFWwindow *window;
ShaderManager *shader_manager;
Scene *scene;

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

void setup_callbacks()
{
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, InputManager::key_callback);
	glfwSetCursorPosCallback(window, InputManager::cursor_position_callback);
	glfwSetMouseButtonCallback(window, InputManager::mouse_button_callback);
	glfwSetScrollCallback(window, InputManager::scroll_callback);
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
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void destroy()
{
	// Free memory here.
	delete(shader_manager);
	delete(scene);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void setup_scene()
{
	scene = new Scene();

	// Load shaders via a shader manager.
	shader_manager->create_shader_program("basic");
	shader_manager->create_shader_program("skybox");
	shader_manager->set_default("basic");

	// Generate geometry and create Mesh + SceneModel.
	Geometry *cube_geometry = GeometryGenerator::generate_cube(1.f);
	Material cube_material;
	cube_material.diffuse = { 1.f, 0.f, 0.f };
	Mesh cube_mesh = { cube_geometry, cube_material, shader_manager->get_default() };
	SceneModel *cube_model = new SceneModel(scene);
	cube_model->add_mesh(cube_mesh);

	// Get root node
	SceneGroup *root = scene->root;
	root->add_child(cube_model);

	// Setup camera
	SceneCamera *camera = scene->camera;
}

int main()
{
	window = Window::create_window(1280, 720, "Greed Island");
	shader_manager = new ShaderManager();
	setup_callbacks();
	setup_opengl_settings();
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

		// scene->render();
		glfwSwapBuffers(window);
	}

	destroy();
	exit(EXIT_SUCCESS);
}