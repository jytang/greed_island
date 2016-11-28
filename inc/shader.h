#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "material.h"
#include "geometry.h"

class Shader
{
public:
    GLuint shader_id;
    glm::mat4 V, P;

    Shader(GLuint shader_id);
    void use();
    void set_VP(glm::mat4 V, glm::mat4 P);
    virtual void set_material(Material m) = 0;
    virtual void draw(Geometry *g, glm::mat4 to_world) = 0;
};
