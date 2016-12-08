#pragma once
#include "scene.h"
class SnowScene :
	public Scene
{
private:
	SceneTransform *map;
	SceneGroup *forest;
public:
	void generate_planes();
	void generate_map();
	void generate_forest();

	void setup();
	GLfloat get_size();
};

