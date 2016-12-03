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

	static Scene *scene;
	static SceneCamera *camera;
	static bool lmb_down;
	static bool rmb_down;
	static bool shift_down;
	static bool vr_on;
	static glm::vec3 last_cursor_pos;
	static bool keys[1024];

	void handle_movement();
	void vr_render();
	void setup_scene();
	void setup_callbacks();
	void setup_opengl();
	void setup_shaders();
	void destroy();
	/* static callbacks */
	static void error_callback(int error, const char* description);
	static void resize_callback(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
public:
	Greed();
	~Greed();
	void go();
	void shadow_pass();
};