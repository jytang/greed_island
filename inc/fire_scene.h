#pragma once
#include "scene.h"
class FireScene :
	public Scene
{
public:
	void generate_planes();
	void generate_map();
	void generate_forest();

	void setup();
	GLfloat get_size();
};

