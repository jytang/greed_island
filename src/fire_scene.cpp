#include "fire_scene.h"
#include "global.h"
#include "colors.h"
#include "util.h"
#include "terrain.h"
#include "geometry_generator.h"
#include "shape_grammar.h"

const GLfloat PLAYER_HEIGHT = Global::PLAYER_HEIGHT;

const GLuint    HEIGHT_MAP_POWER = 8;
const GLuint    HEIGHT_MAP_SIZE = (unsigned int)glm::pow(2, HEIGHT_MAP_POWER) + 1;
const GLint     VILLAGE_DIAMETER = 20.f;
const GLuint    TERRAIN_RESOLUTION = 200;

const GLfloat   SIZE = 30.f * PLAYER_HEIGHT;
const GLfloat   HEIGHT_MAP_MAX = -100.f * PLAYER_HEIGHT;
const GLfloat   HEIGHT_RANDOMNESS_SCALE = 20.f;
const GLfloat	TERRAIN_SMOOTHNESS = 2.0f;
const GLfloat   TERRAIN_SIZE = SIZE;
const GLfloat   TERRAIN_SCALE = 2.f;


GLfloat FireScene::get_size()
{
	return SIZE;
}

void FireScene::setup()
{
	std::cerr << "==============" << std::endl;
	std::cerr << "| FIRE SCENE |" << std::endl;
	std::cerr << "==============" << std::endl;
	light_pos = glm::vec3(0.f, SIZE * 2, SIZE);

	generate_planes();
	generate_map();
}

void FireScene::generate_planes()
{
	/*
	// Curvy beach plane, named Bezier Beach Resort
	Geometry *beach_geo = GeometryGenerator::generate_bezier_plane(SIZE*1.5f, 50, 150, 0.1f, SAND, 0);
	Material beach_material;
	beach_material.diffuse = beach_material.ambient = color::windwaker_sand;
	beach_material.shadows = false;
	Mesh beach_mesh = { beach_geo, beach_material, ShaderManager::get_default(), glm::mat4(1.f) };
	SceneModel *beach_model = new SceneModel(this);
	beach_model->add_mesh(beach_mesh);
	SceneTransform *beach_scale = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(1.0f, 1.0f, 1.0f)));
	SceneTransform *beach_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -0.01f * PLAYER_HEIGHT, 0.0f)));
	beach_scale->add_child(beach_model);
	beach_translate->add_child(beach_scale);
	root->add_child(beach_translate);
	*/
}

void FireScene::generate_map()
{
	std::cerr << "Generating Map...";

	SceneTransform *map = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(TERRAIN_SCALE, 1.f, TERRAIN_SCALE)));
	root->add_child(map);

	height_map = Terrain::generate_height_map(HEIGHT_MAP_SIZE, HEIGHT_MAP_MAX, VILLAGE_DIAMETER, HEIGHT_RANDOMNESS_SCALE, false, true, TERRAIN_SMOOTHNESS, 0);
	
	Geometry *sand_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, -100.f, 100.f, false, SAND, height_map);
	Material sand_material;
	sand_material.diffuse = sand_material.ambient = color::windwaker_sand;
	Mesh sand_mesh = { sand_geo, sand_material, ShaderManager::get_default(), glm::mat4(1.f) };

	SceneModel *terrain_model = new SceneModel(this);
	terrain_model->add_mesh(sand_mesh);
	map->add_child(terrain_model);
	std::cerr << "OK." << std::endl;
}