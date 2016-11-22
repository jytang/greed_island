#include "main.h"
#include "window.h"
#include "input_manager.h"
#include "shader_manager.h"

GLFWwindow *window;
ShaderManager *shader_manager;
// Scene *scene;

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

void setup_callbacks()
{
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, InputManager::key_callback);
	glfwSetCursorPosCallback(window, InputManager::cursor_position_callback);
	glfwSetMouseButtonCallback(window, InputManager::mouse_button_callback);
	glfwSetScrollCallback(window, InputManager::scroll_callback);
	glfwSetFramebufferSizeCallback(window, Window::resize_callback);
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

	glfwDestroyWindow(window);
	glfwTerminate();
}

void setup_scene()
{
	// Load shaders via a shader manager.
	shader_manager->create_shader_program("basic");
	shader_manager->create_shader_program("skybox");
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

		// scene->render();
		glfwSwapBuffers(window);
	}

	destroy();
	exit(EXIT_SUCCESS);
}