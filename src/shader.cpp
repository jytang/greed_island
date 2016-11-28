#include "shader.h"

Shader::Shader(GLuint shader_id)
    : shader_id(shader_id) {}

void Shader::use()
{
    glUseProgram(shader_id);
}

void Shader::set_VP(glm::mat4 V, glm::mat4 P)
{
    this->V = V;
    this->P = P;
}
