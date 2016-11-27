#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h> // Remove this line in future projects
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Util
{
    public:
        static void print_vec3(glm::vec3 v);
        static void print_mat4(glm::mat4 m);
        static glm::mat4 calc_bezier_mat(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
        static glm::vec3 trackball_position(double x_pos, double y_pos, int width, int height);
};

#endif
