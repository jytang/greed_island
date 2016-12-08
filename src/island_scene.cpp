#include "island_scene.h"
#include "global.h"
#include "colors.h"
#include "util.h"
#include "terrain.h"
#include "tree.h"
#include "scene_animation.h"
#include "geometry_generator.h"
#include "shape_grammar.h"

#include <iostream>

const GLfloat PLAYER_HEIGHT = Global::PLAYER_HEIGHT;

// Procedural generation parameters
// Hardcoded
const GLuint    HEIGHT_MAP_POWER = 8;
const GLuint    HEIGHT_MAP_SIZE = (unsigned int)glm::pow(2, HEIGHT_MAP_POWER) + 1;
const GLint     VILLAGE_DIAMETER = (int)(0.23f * HEIGHT_MAP_SIZE);
const GLuint    TERRAIN_RESOLUTION = 200;
const GLuint    NUM_TREES = 100;
const GLfloat   PERCENT_TREE_ANIM = 0.9f;
const GLfloat   TREE_SCALE = 1.5f;
const GLint		NUM_BUILDINGS = 5;

// Dependent on island size/player height
const GLfloat   ISLAND_SIZE = 30.f * PLAYER_HEIGHT;
const GLfloat   HEIGHT_MAP_MAX = 10.f * PLAYER_HEIGHT;
const GLfloat   HEIGHT_RANDOMNESS_SCALE = HEIGHT_MAP_MAX;
const GLfloat   TERRAIN_SIZE = ISLAND_SIZE / 5.f;
const GLfloat   TERRAIN_SCALE = ISLAND_SIZE / (TERRAIN_SIZE / 2);
const GLfloat	VILLAGE_DIAMETER_TRUE = ((float)VILLAGE_DIAMETER / HEIGHT_MAP_SIZE) * TERRAIN_SIZE * TERRAIN_SCALE;
const GLfloat   BEACH_HEIGHT = ISLAND_SIZE / 60.f;
const GLfloat   PATH_WIDTH = ISLAND_SIZE / 7.f;
const GLfloat   FOREST_RADIUS = ISLAND_SIZE / 1.1f;
const GLfloat   FOREST_INNER_CIRCLE = VILLAGE_DIAMETER_TRUE / 2 + ISLAND_SIZE / 10.f;
const GLfloat   WATER_SCALE = ISLAND_SIZE * 4;

// Camera and movement
const GLint     CAM_OFFSET = ISLAND_SIZE / 30.f;
const GLfloat   EDGE_LEEWAY = ISLAND_SIZE / 6.f;
const GLfloat   MOVE_BOUNDS = ISLAND_SIZE + EDGE_LEEWAY;
const GLfloat   EDGE_THRESH = ISLAND_SIZE / 30.f;
const GLfloat   HELI_HEIGHT = ISLAND_SIZE;
const GLfloat   HELI_SPEED = 0.5f;
const GLfloat   HELI_SLOW_THRESH = 20.f;

// Miniature Parameters
const GLfloat SMALL_ROT_SPEED = 0.3f;
const GLfloat SMALL_MAP_SCALE = 0.15f;
const GLuint  NUM_SMALL_TREES = 6;
const GLfloat SMALL_TREE_SCALE = 0.1f;
const GLfloat SMALL_FOREST_RADIUS = 1.5f;
const GLuint  NUM_SMALL_BUILDINGS = 5;
const GLfloat SMALL_BUILDING_SCALE = 0.1f;
const GLfloat SMALL_VILLAGE_RADIUS = 1.5f;

SceneTransform *cube_translate;

GLfloat IslandScene::get_size()
{
	return ISLAND_SIZE;
}

void IslandScene::setup()
{
	std::cerr << "================" << std::endl;
	std::cerr << "| ISLAND SCENE |" << std::endl;
	std::cerr << "================" << std::endl;
	// Initial position
	camera->cam_pos = glm::vec3(0.f, 0.f, ISLAND_SIZE - CAM_OFFSET);
	camera->recalculate();
	// Light(s)
	light_pos = glm::vec3(0.f, ISLAND_SIZE * 2, ISLAND_SIZE);

	/* BEGIN ACTUAL OBJECTS TO BE RENDERED */
	// "Starter" geometry kit
	cylinder_geo = GeometryGenerator::generate_cylinder(0.25f, 2.f, 3, false);
	diamond_geo = GeometryGenerator::generate_sphere(2.f, 3);
	cube_geo = GeometryGenerator::generate_cube(1.f, true);

	// Generate everything.
	generate_planes();
	generate_map();
	generate_forest();
	generate_village();
	generate_miniatures();
}

void IslandScene::generate_planes()
{
	// Infinite water plane stretching to the horizon
	Geometry *plane_geo = GeometryGenerator::generate_plane(1.f, WATER);
	Material water_material;
	water_material.diffuse = water_material.ambient = color::ocean_blue;
	water_material.shadows = false;
	Mesh water_mesh = { plane_geo, water_material, ShaderManager::get_default(), glm::mat4(1.f) };
	SceneModel *water_model = new SceneModel(this);
	water_model->add_mesh(water_mesh);
	SceneTransform *water_scale = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(WATER_SCALE, 1.0f, WATER_SCALE)));
	SceneTransform *water_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -0.5f * PLAYER_HEIGHT, 0.0f)));
	water_scale->add_child(water_model);
	water_translate->add_child(water_scale);
	root->add_child(water_translate);

	// Curvy beach plane, named Bezier Beach Resort
	Geometry *beach_geo = GeometryGenerator::generate_bezier_plane(ISLAND_SIZE*1.5f, 50, 150, 0.1f, SAND, 0);
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

void IslandScene::generate_map()
{
	std::cerr << "Generating Map...";

	if (!map)
	{
		map = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(TERRAIN_SCALE, 1.f, TERRAIN_SCALE)));
		root->add_child(map);
	}
	else {
		map->remove_all();
	}

	height_map = Terrain::generate_height_map(HEIGHT_MAP_SIZE, HEIGHT_MAP_MAX, VILLAGE_DIAMETER, HEIGHT_RANDOMNESS_SCALE, true, 0);
	float cam_height = Terrain::height_lookup(0.f, ISLAND_SIZE - CAM_OFFSET, ISLAND_SIZE * 2, height_map);
	camera->cam_pos.y = cam_height + PLAYER_HEIGHT;
	camera->recalculate();

	// Mainland
	// Second Parameter Below is Resolution^2 of Island, LOWER TO RUN FASTER
	land_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, BEACH_HEIGHT, HEIGHT_MAP_MAX * 0.8f, false, GRASS, height_map);
	Material land_material;
	land_material.diffuse = land_material.ambient = color::windwaker_green;
	Mesh land_mesh = { land_geo, land_material, ShaderManager::get_default(), glm::mat4(1.f) };

	// Plateau/village
	plateau_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, HEIGHT_MAP_MAX * 0.8f, HEIGHT_MAP_MAX, false, ROCK, height_map);
	Material plateau_material;
	plateau_material.diffuse = plateau_material.ambient = color::bone_white;
	Mesh plateau_mesh = { plateau_geo, plateau_material, ShaderManager::get_default(), glm::mat4(1.f) };

	// Beachfront
	sand_geo = GeometryGenerator::generate_terrain(TERRAIN_SIZE, TERRAIN_RESOLUTION, 0.0f, BEACH_HEIGHT, true, SAND, height_map);
	Material sand_material;
	sand_material.diffuse = sand_material.ambient = color::windwaker_sand;
	Mesh sand_mesh = { sand_geo, sand_material, ShaderManager::get_default(), glm::mat4(1.f) };

	SceneModel *terrain_model = new SceneModel(this);
	terrain_model->add_mesh(land_mesh);
	terrain_model->add_mesh(sand_mesh);
	terrain_model->add_mesh(plateau_mesh);
	map->add_child(terrain_model);
	std::cerr << "OK." << std::endl;
}

void IslandScene::generate_forest()
{
	std::cerr << "Generating Forest...";

	if (!forest)
	{
		forest = new SceneGroup(this);
		root->add_child(forest);
	}
	else {
		forest->remove_all();
	}

	glm::vec3 leaf_colors[] = { color::olive_green, color::olive_green, color::olive_green, color::autumn_orange, color::purple, color::bone_white, color::indian_red };
	glm::vec3 branch_colors[] = { color::brown, color::wood_saddle, color::wood_sienna, color::wood_tan, color::wood_tan_light };
	Material branch_material, leaf_material;
	for (int i = 0; i < NUM_TREES; ++i) {
		if (i % 49 == 0)
			std::cerr << " . ";

		// Randomise colours, animation, location.
		leaf_material.diffuse = leaf_material.ambient = leaf_colors[(int)Util::random(0, 7)];
		branch_material.diffuse = branch_material.ambient = branch_colors[(int)Util::random(0, 5)];
		bool animated = false;
		if (i % (NUM_TREES / (int)(NUM_TREES*PERCENT_TREE_ANIM)) == 0)
			animated = true;
		float x, z;
		do {
			float angle = Util::random(0, 360);
			float distance = Util::random(FOREST_INNER_CIRCLE, FOREST_RADIUS);

			x = glm::cos(glm::radians(angle)) * distance;
			z = glm::sin(glm::radians(angle)) * distance;
		} while (Util::within_rect(glm::vec2(x, z), glm::vec2(-PATH_WIDTH / 2, FOREST_RADIUS), glm::vec2(PATH_WIDTH / 2, 0)));
		float y = Terrain::height_lookup(x, z, ISLAND_SIZE * 2, height_map);
		glm::vec3 location = { x, y, z };

		// Ugly code below.
		SceneGroup *tree = Tree::generate_tree(this, cylinder_geo, diamond_geo, 7, 1, 20.f, 2.f, branch_material, leaf_material, animated, location, 0);
		((SceneModel *)(tree->children[0]))->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		if (animated)
			((SceneModel *)((SceneAnimation *)(tree->children[1]))->children[0])->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		else
			((SceneModel *)(tree->children[1]))->meshes[0].to_world = glm::translate(glm::mat4(1.f), location) * glm::scale(glm::mat4(1.f), glm::vec3(TREE_SCALE));
		// Phew.
		forest->add_child(tree);
	}
	std::cerr << "OK." << std::endl;
}

void IslandScene::generate_village()
{
	std::cerr << "Generating Village...";

	if (!village)
	{
		village = new SceneGroup(this);
		root->add_child(village);
		out_house = new SceneTransAnim(this, glm::vec3(0.f), glm::vec3(0.f, -0.5f, 0.f), false);
		root->add_child(out_house);

		cube_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.2f*PLAYER_HEIGHT, 0.f)));
	}
	else {
		village->remove_all();
		out_house->remove_all();
		cube_translate->remove_all();
	}

	for (int i = 0; i < NUM_BUILDINGS; ++i)
	{
		float angle = ((360.f / (NUM_BUILDINGS)) * i) - 54.f; //Circles around, starting from the right
		float distance = VILLAGE_DIAMETER_TRUE / 3.f;

		float x = glm::cos(glm::radians(angle)) * distance;
		float z = -glm::sin(glm::radians(angle)) * distance;

		float y = Terrain::height_lookup(x, z, ISLAND_SIZE * 2, this->height_map);
		glm::vec3 location = { x, y, z };

		float rot = glm::radians(-90.f + angle);

		SceneModel *building = ShapeGrammar::generate_building(this, true, 0);
		SceneTransform *building_rotate = new SceneTransform(this, glm::rotate(glm::mat4(1.f), rot, glm::vec3(0.f, 1.f, 0.f)));
		SceneTransform *building_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), location));
		building_rotate->add_child(building);
		building_translate->add_child(building_rotate);

		Material cube_mat;
		cube_mat.diffuse = cube_mat.ambient = color::red;
		Mesh cube_mesh = { cube_geo, cube_mat, ShaderManager::get_default(), glm::mat4(1.f) };
		SceneModel *cube_model = new SceneModel(this);
		cube_model->add_mesh(cube_mesh);
		SceneTransform *cube_scale = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(0.1f*PLAYER_HEIGHT)));
		cube_scale->add_child(cube_model);
		cube_translate->add_child(cube_scale);

		if (i == 0)
		{
			building_rotate->add_child(cube_translate);
			out_house->add_child(building_translate);
			out_height = y;
			out_point = glm::vec2(x, z);
		}
		else if (i == NUM_BUILDINGS - 1)
		{
			building_rotate->add_child(cube_translate);
			in_house = building_translate;
			in_height = y;
			in_area[0] = glm::vec2(x-Global::TRIGGER_HALF_LEN, z+ Global::TRIGGER_HALF_LEN);
			in_area[1] = glm::vec2(x+ Global::TRIGGER_HALF_LEN, z- Global::TRIGGER_HALF_LEN);
			in_point = glm::vec2(x, z);
			village->add_child(building_translate);
		}
		else {
			village->add_child(building_translate);
		}
	}
	std::cerr << "OK." << std::endl;
}

void IslandScene::generate_miniatures()
{
	std::cerr << "Generating Miniatures...";

	glm::vec3 pivot_pt(0.f, HEIGHT_MAP_MAX - 2.f, -8.f);
	float building_positions[NUM_BUILDINGS]; // positions starting from rightmost
	for (int i = 0; i < NUM_BUILDINGS; ++i)
		building_positions[i] = glm::radians((i - (NUM_BUILDINGS / 2)) * (360.f / NUM_BUILDINGS));

	// Small terrain
	generate_small_map();
	SceneTransform *small_map_scale = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(SMALL_MAP_SCALE, SMALL_MAP_SCALE / 10.f, SMALL_MAP_SCALE)));
	SceneAnimation *small_map_anim = new SceneAnimation(this, 0.f, FLT_MAX, 0.f, SMALL_ROT_SPEED, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
	SceneTransform *small_map_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), pivot_pt));
	SceneTransform *small_map_rotate = new SceneTransform(this, glm::rotate(glm::mat4(1.f), building_positions[1], glm::vec3(0.f, 1.f, 0.f)));
	small_map_scale->add_child(small_map_model);
	small_map_anim->add_child(small_map_scale);
	small_map_translate->add_child(small_map_anim);
	small_map_rotate->add_child(small_map_translate);
	root->add_child(small_map_rotate);

	// Small trees
	generate_small_forest();
	SceneTransform *small_forest_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), pivot_pt));
	SceneTransform *small_forest_rotate = new SceneTransform(this, glm::rotate(glm::mat4(1.f), building_positions[2], glm::vec3(0.f, 1.f, 0.f)));
	small_forest_translate->add_child(small_forest);
	small_forest_rotate->add_child(small_forest_translate);
	root->add_child(small_forest_rotate);

	// Small village
	generate_small_village();
	SceneTransform *small_village_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), pivot_pt));
	SceneTransform *small_village_rotate = new SceneTransform(this, glm::rotate(glm::mat4(1.f), building_positions[3], glm::vec3(0.f, 1.f, 0.f)));
	small_village_translate->add_child(small_village);
	small_village_rotate->add_child(small_village_translate);
	root->add_child(small_village_rotate);

	std::cerr << "OK." << std::endl;
}

void IslandScene::generate_small_map()
{
	if (!small_map_model)
		small_map_model = new SceneModel(this);
	else
		small_map_model->meshes.clear();

	Material small_land_material;
	small_land_material.diffuse = small_land_material.ambient = color::windwaker_green;
	small_land_material.shadows = false;
	Mesh small_land_mesh = { land_geo, small_land_material, ShaderManager::get_default(), glm::mat4(1.f) };
	small_land_mesh.no_culling = true;
	Material small_plateau_material;
	small_plateau_material.diffuse = small_plateau_material.ambient = color::bone_white;
	small_plateau_material.shadows = false;
	Mesh small_plateau_mesh = { plateau_geo, small_plateau_material, ShaderManager::get_default(), glm::mat4(1.f) };
	small_plateau_mesh.no_culling = true;
	Material small_sand_material;
	small_sand_material.diffuse = small_sand_material.ambient = color::windwaker_sand;
	small_sand_material.shadows = false;
	Mesh small_sand_mesh = { sand_geo, small_sand_material, ShaderManager::get_default(), glm::mat4(1.f) };
	small_sand_mesh.no_culling = true;
	small_map_model->add_mesh(small_land_mesh);
	small_map_model->add_mesh(small_sand_mesh);
	small_map_model->add_mesh(small_plateau_mesh);
}

void IslandScene::generate_small_forest()
{
	if (!small_forest)
		small_forest = new SceneGroup(this);
	else
		small_forest->remove_all();

	float seed = Util::random(0, 1000.f);

	Material branch_material, leaf_material;
	glm::vec3 leaf_colors[] = { color::olive_green, color::olive_green, color::olive_green, color::autumn_orange, color::purple, color::bone_white, color::indian_red };
	glm::vec3 branch_colors[] = { color::brown, color::wood_saddle, color::wood_sienna, color::wood_tan, color::wood_tan_light };
	leaf_material.diffuse = leaf_material.ambient = leaf_colors[(int)Util::random(0, 7)];
	branch_material.diffuse = branch_material.ambient = branch_colors[(int)Util::random(0, 5)];
	branch_material.shadows = false;
	leaf_material.shadows = false;
	for (int i = 0; i < NUM_SMALL_TREES; ++i)
	{
		SceneGroup *small_tree = Tree::generate_tree(this, cylinder_geo, diamond_geo, 2 + i, 0, 20.f, 2.f, branch_material, leaf_material, false, glm::vec3(0.f), seed);
		SceneTransform *small_tree_scale = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(SMALL_TREE_SCALE)));
		SceneAnimation *small_tree_anim = new SceneAnimation(this, 0.f, FLT_MAX, 0.f, SMALL_ROT_SPEED, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		// Display in a half-circle arc.
		float x = SMALL_FOREST_RADIUS*glm::cos(glm::radians(180.f / NUM_SMALL_TREES * (NUM_SMALL_TREES - i)));
		float z = -SMALL_FOREST_RADIUS*glm::sin(glm::radians(180.f / NUM_SMALL_TREES * i));
		SceneTransform *small_tree_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), glm::vec3(x, 0, z)));
		small_tree_scale->add_child(small_tree);
		small_tree_anim->add_child(small_tree_scale);
		small_tree_translate->add_child(small_tree_anim);
		small_forest->add_child(small_tree_translate);
	}
}

void IslandScene::generate_small_village()
{
	if (!small_village)
		small_village = new SceneGroup(this);
	else
		small_village->remove_all();

	for (int i = 0; i < NUM_SMALL_BUILDINGS; ++i)
	{
		SceneModel *building = ShapeGrammar::generate_building(this, false, 0);
		SceneTransform *small_building_scale = new SceneTransform(this, glm::scale(glm::mat4(1.f), glm::vec3(SMALL_BUILDING_SCALE)));
		// Display in a half-circle arc.
		float angle = glm::radians(180.f / NUM_SMALL_BUILDINGS * i);
		SceneTransform *small_building_rot = new SceneTransform(this, glm::rotate(glm::mat4(1.f), glm::radians(-90.f + glm::degrees(angle)), glm::vec3(0.f, 1.f, 0.f)));
		float x = SMALL_VILLAGE_RADIUS*glm::cos(angle);
		float z = -SMALL_VILLAGE_RADIUS*glm::sin(angle);
		SceneTransform *small_building_translate = new SceneTransform(this, glm::translate(glm::mat4(1.f), glm::vec3(x, 0, z)));
		small_building_scale->add_child(building);
		small_building_rot->add_child(small_building_scale);
		small_building_translate->add_child(small_building_rot);
		small_village->add_child(small_building_translate);
	}
}

void IslandScene::handle_helicopter()
{
	float x = MOVE_BOUNDS*glm::cos(glm::radians(helicopter_angle));
	float z = -MOVE_BOUNDS*glm::sin(glm::radians(helicopter_angle));
	float y = HELI_HEIGHT - helicopter_angle*0.1f; // spiral down....

	if (y <= PLAYER_HEIGHT)
		return;

	float displacement = HELI_SPEED;

	if (y <= PLAYER_HEIGHT + HELI_SLOW_THRESH)
	{
		float diff = PLAYER_HEIGHT + HELI_SLOW_THRESH - y;
		displacement *= glm::max((HELI_SLOW_THRESH - diff) / HELI_SLOW_THRESH, 0.01f);
	}

	helicopter_angle += displacement;

	camera->cam_pos = glm::vec3(x, y, z);
	camera->recalculate();
}