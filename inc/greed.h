#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stdlib.h>
#include <iostream>

#include "shader_manager.h"
#include "scene.h"

#include "greed_vr.h"

class Greed
{
private:
	GLFWwindow *window;

	void handle_movement();
	void handle_movement_vr();
	void vr_render();
	void setup_scenes();
	void setup_callbacks();
	void setup_opengl();
	void setup_shaders();
	void destroy();
	static void next_skybox();
	static void change_scene(Scene *);
	static void next_scene();
	/* static callbacks */
	static void error_callback(int error, const char* description);
	static void resize_callback(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	static void vr_interaction_check();
public:
	Greed();
	~Greed();
	void go();
	void shadow_pass();
};