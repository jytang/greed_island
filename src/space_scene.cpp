#include "space_scene.h"
#include "global.h"
#include "colors.h"
#include "util.h"
#include "terrain.h"
#include "geometry_generator.h"
#include "shape_grammar.h"

const GLfloat PLAYER_HEIGHT = Global::PLAYER_HEIGHT;

const GLuint    HEIGHT_MAP_POWER = 8;
const GLuint    HEIGHT_MAP_SIZE = (unsigned int)glm::pow(2, HEIGHT_MAP_POWER) + 1;
const GLint     VILLAGE_DIAMETER = 120;
const GLuint    TERRAIN_RESOLUTION = 200;

const GLfloat   SIZE = 80.f * PLAYER_HEIGHT;
const GLfloat   WATER_SCALE = SIZE * 4;
const GLfloat   HEIGHT_MAP_MAX = 30.f * PLAYER_HEIGHT;
const GLfloat   HEIGHT_RANDOMNESS_SCALE = HEIGHT_MAP_MAX;
const GLfloat	TERRAIN_SMOOTHNESS = 0.1f;
const GLfloat   TERRAIN_SIZE = SIZE;
const GLfloat   TERRAIN_SCALE = 2.f;

GLfloat SpaceScene::get_size()
{
	return SIZE;
}

void SpaceScene::setup()
{
	std::cerr << "===============" << std::endl;
	std::cerr << "| SPACE SCENE |" << std::endl;
	std::cerr << "===============" << std::endl;
	light_pos = glm::vec3(0.f, SIZE * 2, SIZE);

	generate_planes();
	generate_map();

	Geometry *cube_geo = GeometryGenerator::generate_cube(1.f, true);
	Material cube_mat;
	cube_mat.diffuse = cube_mat.ambient = color::red;
	Mesh cube_mesh = { cube_geo, cube_mat, ShaderManager::get_default(), glm::mat4(1.f) };
	SceneModel *cube_model = new SceneModel(this);
	cube_model->add_mesh(cube_mesh);
	SceneTransform *cube_scale = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(0.4f*PLAYER_HEIGHT)));
	cube_scale->add_child(cube_model);
	SceneTransform *cube_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.8f*PLAYER_HEIGHT, 0.f)));
	cube_translate->add_child(cube_scale);

	// Make entry portal.
	float x = 40.f;
	float z = -20.f;
	float y = Terrain::height_lookup(x, z, SIZE * 2, height_map);
	glm::vec3 location = { x, y, z };
	SceneModel *building = ShapeGrammar::generate_building(this, true, 0);
	SceneTransform *building_rotate = new SceneTransform(this, glm::rotate(glm::mat4(1.f), glm::radians(27.f), glm::vec3(0.f, 1.f, 0.f)));
	SceneTransform *building_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), location));
	building_rotate->add_child(building);
	building_rotate->add_child(cube_translate);
	building_translate->add_child(building_rotate);
	in_house = building_translate;
	in_height = y;
	in_area[0] = glm::vec2(x - Global::TRIGGER_HALF_LEN, z + Global::TRIGGER_HALF_LEN);
	in_area[1] = glm::vec2(x + Global::TRIGGER_HALF_LEN, z - Global::TRIGGER_HALF_LEN);
	in_point = glm::vec2(x, z);

	root->add_child(in_house);
}

void SpaceScene::generate_planes()
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

void SpaceScene::generate_map()
{
	std::cerr << "Generating Map...";

	SceneTransform *map = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(TERRAIN_SCALE, 1.f, TERRAIN_SCALE)));
	root->add_child(map);

	height_map = Terrain::generate_height_map(HEIGHT_MAP_SIZE, 0.f, VILLAGE_DIAMETER, HEIGHT_RANDOMNESS_SCALE, false, false, TERRAIN_SMOOTHNESS, 0);
	
	Geometry *sand_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, -HEIGHT_MAP_MAX, HEIGHT_MAP_MAX, false, ROCK, height_map);
	Material sand_material;
	sand_material.diffuse = sand_material.ambient = color::windwaker_sand;
	Mesh sand_mesh = { sand_geo, sand_material, ShaderManager::get_default(), glm::mat4(1.f) };

	SceneModel *terrain_model = new SceneModel(this);
	terrain_model->add_mesh(sand_mesh);
	map->add_child(terrain_model);
	std::cerr << "OK." << std::endl;
}