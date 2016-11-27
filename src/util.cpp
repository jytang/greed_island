#include "Util.h"

void Util::print_vec3(glm::vec3 v)
{
    std::cerr << v.x << ", " << v.y << ", " << v.z << std::endl;
}

void Util::print_mat4(glm::mat4 m)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            std::cerr << m[j][i] << "\t";
        }
        std::cerr << std::endl;
    }
}

glm::mat4 Util::calc_bezier_mat(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
    // column-wise
    float points[16] = {
        p0.x, p0.y, p0.z, 0.f,
        p1.x, p1.y, p1.z, 0.f,
        p2.x, p2.y, p2.z, 0.f,
        p3.x, p3.y, p3.z, 0.f
    };
    glm::mat4 G;
    memcpy(glm::value_ptr(G), points, sizeof(points));

    float Bcoeffs[16] = {
        -1.f, 3.f, -3.f, 1.f,
        3.f, -6.f, 3.f, 0.f,
        -3.f, 3.f, 0.f, 0.f,
        1.f, 0.f, 0.f, 0.f
    };
    glm::mat4 B;
    memcpy(glm::value_ptr(B), Bcoeffs, sizeof(Bcoeffs));

    glm::mat4 ret = G*B;
    ret[3][3] = 1.f;
    return ret;
}

glm::vec3 Util::trackball_position(double x_pos, double y_pos, int width, int height)
{
    glm::vec3 ret(0.0f);
    float d;
    ret.x = (float) (2 * x_pos - width) / width;
    ret.y = (float) (height - 2.0 * y_pos) / height;
    ret.z = 0.0;
    d = glm::length(ret);
    d = (d<1.f) ? d : 1.f;
    ret.z = (float) glm::sqrt(1.001 - d*d);
    ret = glm::normalize(ret);
    return ret;
}
