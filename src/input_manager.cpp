#include "input_manager.h"

void InputManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
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

void InputManager::cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos)
{

}

void InputManager::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}

void InputManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}