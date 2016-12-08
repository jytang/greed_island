#include "fire_scene.h"
#include "global.h"
#include "colors.h"
#include "util.h"
#include "terrain.h"
#include "geometry_generator.h"
#include "shape_grammar.h"
#include "tree.h"

const GLfloat PLAYER_HEIGHT = Global::PLAYER_HEIGHT;

const GLuint    HEIGHT_MAP_POWER = 8;
const GLuint    HEIGHT_MAP_SIZE = (unsigned int)glm::pow(2, HEIGHT_MAP_POWER) + 1;
const GLint     VILLAGE_DIAMETER = (int)(0.23f * HEIGHT_MAP_SIZE);
const GLuint    TERRAIN_RESOLUTION = 200;
const GLuint    NUM_TREES = 100;
const GLfloat   PERCENT_TREE_ANIM = 0.9f;
const GLfloat   TREE_SCALE = 1.5f;

const GLfloat   SIZE = 30.f * PLAYER_HEIGHT;
const GLfloat   HEIGHT_MAP_MAX = 10.f * PLAYER_HEIGHT;
const GLfloat   HEIGHT_RANDOMNESS_SCALE = HEIGHT_MAP_MAX;
const GLfloat	TERRAIN_SMOOTHNESS = 1.2f;
const GLfloat   TERRAIN_SIZE = SIZE;
const GLfloat   TERRAIN_SCALE = 2.f;
const GLfloat	VILLAGE_DIAMETER_TRUE = ((float)VILLAGE_DIAMETER / HEIGHT_MAP_SIZE) * TERRAIN_SIZE * TERRAIN_SCALE;
const GLfloat   PATH_WIDTH = SIZE / 7.f;
const GLfloat   FOREST_RADIUS = SIZE / 1.1f;
const GLfloat   FOREST_INNER_CIRCLE = VILLAGE_DIAMETER_TRUE / 2 + SIZE / 10.f;

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
	generate_forest();
}

void FireScene::generate_planes()
{	
	// Curvy beach plane, named Bezier Beach Resort
	Geometry *beach_geo = GeometryGenerator::generate_bezier_plane(SIZE*1.5f, 50, 150, 0.1f, OBSIDIAN, 0);
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
}

void FireScene::generate_forest()
{
	std::cerr << "Generating Forest...";

	SceneGroup *forest = new SceneGroup(this);
	root->add_child(forest);

	Geometry *cylinder_geo = GeometryGenerator::generate_cylinder(0.25f, 2.f, 3, false);
	Geometry *diamond_geo = GeometryGenerator::generate_sphere(2.f, 3);

	glm::vec3 leaf_colors[] = { color::black };
	glm::vec3 branch_colors[] = { color::black };
	Material branch_material, leaf_material;
	for (int i = 0; i < NUM_TREES; ++i)
	{
		if (i % 49 == 0)
			std::cerr << " . ";

		// Randomise colours, animation, location.
		leaf_material.diffuse = leaf_material.ambient = leaf_colors[0];
		branch_material.diffuse = branch_material.ambient = branch_colors[0];
		float x, z;
		do
		{
			float angle = Util::random(0, 360);
			float distance = Util::random(FOREST_INNER_CIRCLE, FOREST_RADIUS);

			x = glm::cos(glm::radians(angle)) * distance;
			z = glm::sin(glm::radians(angle)) * distance;
		} while (Util::within_rect(glm::vec2(x, z), glm::vec2(-PATH_WIDTH / 2, FOREST_RADIUS), glm::vec2(PATH_WIDTH / 2, 0)));
		float y = Terrain::height_lookup(x, z, SIZE * 2, height_map);
		glm::vec3 location = { x, y, z };

		SceneGroup *tree = Tree::generate_tree(this, cylinder_geo, diamond_geo, 7, 0, 20.f, 2.f, branch_material, leaf_material, false, location, 0);
		((SceneModel *)(tree->children[0]))->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));

		forest->add_child(tree);
	}
	std::cerr << "OK." << std::endl;
}

void FireScene::generate_map()
{
	std::cerr << "Generating Map...";

	SceneTransform *map = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(TERRAIN_SCALE, 1.f, TERRAIN_SCALE)));
	root->add_child(map);

	height_map = Terrain::generate_height_map(HEIGHT_MAP_SIZE, HEIGHT_MAP_MAX, VILLAGE_DIAMETER, HEIGHT_RANDOMNESS_SCALE, true, false, TERRAIN_SMOOTHNESS, 0);
	
	Geometry *sand_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, -20.f, 1000.f, false, OBSIDIAN, height_map);
	Material sand_material;
	sand_material.diffuse = sand_material.ambient = color::windwaker_sand;
	Mesh sand_mesh = { sand_geo, sand_material, ShaderManager::get_default(), glm::mat4(1.f) };

	SceneModel *terrain_model = new SceneModel(this);
	terrain_model->add_mesh(sand_mesh);
	map->add_child(terrain_model);
	std::cerr << "OK." << std::endl;
}