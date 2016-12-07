#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <time.h>

#include "scene_model.h"
#include "scene_transform.h"
#include "colors.h"
#include "shader_manager.h"

#define BOX 0
#define CYLINDER 1
#define HEMISPHERE 2
#define PYRAMID 3
#define CONE 4
#define PLANE 5

class ShapeGrammar
{
private:
	static void create_base(SceneModel *);
	static void add_box(SceneModel *, float, float);
	static void add_cylinder(SceneModel *, float, float, float offset = 0.0f);
	static void add_hemisphere(SceneModel *, float, float, float offset = 0.0f);
	static void add_pyramid(SceneModel *, float, float);
	static void add_cone(SceneModel *, float, float, float offset = 0.0f);
	static void add_plane(SceneModel *, float, float, int, float offset = 0.0f);
	static Material random_material();
public:
	static SceneModel *generate_building(Scene *, bool, int);
};

