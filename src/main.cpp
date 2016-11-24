#include "main.h"
#include "window.h"
#include "input_manager.h"
#include "shader_manager.h"
#include "geometry_generator.h"
#include "scene_model.h"
#include "scene_transform.h"
#include "scene_camera.h"

GLFWwindow *window;
ShaderManager *shader_manager;
SceneGroup *world;
SceneCamera *camera;

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

void resize_callback(GLFWwindow* window, int width, int height)
{
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		//P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		//V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	}
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
	delete(world);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void setup_scene()
{
	world = new SceneGroup();

	// Load shaders via a shader manager.
	shader_manager->create_shader_program("basic");
	shader_manager->create_shader_program("skybox");
	shader_manager->set_default("basic");

	// Generate geometry and create Mesh + SceneModel.
	Geometry *cube_geometry = GeometryGenerator::generate_cube(1.f);
	Material cube_material;
	cube_material.diffuse = { 1.f, 0.f, 0.f };
	Mesh cube_mesh = { cube_geometry, cube_material, shader_manager->get_default() };
	SceneModel *cube_model = new SceneModel();
	cube_model->add_mesh(cube_mesh);

	world->add_child(cube_model);

	// Setup camera
	camera = new SceneCamera(glm::mat4(1.f));
	world->add_child(camera);
}

int main()
{
	window = Window::create_window(1600, 900, "Test");
	shader_manager = new ShaderManager();
	setup_callbacks();
	setup_opengl_settings();
	setup_scene();
	// Setup VR.

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// world->render();
		glfwSwapBuffers(window);
	}

	destroy();
	exit(EXIT_SUCCESS);
}