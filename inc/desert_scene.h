#pragma once
#include "scene.h"
class DesertScene :
	public Scene
{
public:
	void generate_planes();
	void generate_map();

	void setup();
	GLfloat get_size();
};

