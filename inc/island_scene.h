#pragma once
#include "scene.h"
#include "scene_group.h"
#include "scene_transform.h"
#include "scene_model.h"
#include "geometry.h"

class IslandScene :
	public Scene
{
private:
	SceneGroup *forest;
	SceneTransform *map;
	SceneGroup *village;
	SceneModel *small_map_model;
	SceneGroup *small_forest;
	SceneGroup *small_village;
	Geometry *cylinder_geo;
	Geometry *diamond_geo;
	Geometry *cube_geo;
	Geometry *land_geo;
	Geometry *plateau_geo;
	Geometry *sand_geo;
public:
	float helicopter_angle;

	void handle_helicopter();

	void generate_planes();
	void generate_map();
	void generate_forest();
	void generate_village();
	void generate_miniatures();
	void generate_small_map();
	void generate_small_forest();
	void generate_small_village();

	void setup();
	GLfloat get_size();
};

