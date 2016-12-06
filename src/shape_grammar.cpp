#include "shape_grammar.h"
#include "util.h"

const float DOOR_HEIGHT = 25.0f; //Should be larger than size of human
const float DOOR_WIDTH = 18.0f;
const float MIN_HEIGHT = 26.0f; //Random Temporary Values
const float MAX_HEIGHT = 35.0f;
const float MIN_WIDTH = 80.0f;
const float MAX_WIDTH = 90.0f;
const float MIN_DIAMETER = 80.0f;
const float MAX_DIAMETER = 90.0f;
const int NUM_DIVISIONS = 15;

//Main Generation Function
SceneModel *ShapeGrammar::generate_building(Scene * scene, int seed)
{
	SceneModel *building = new SceneModel(scene);

	//Set seed if necessary
	if (seed != 0)
		srand(seed);

	//Recursively adds levels to the building, starting with the base
	create_base(building); 

	return building;
}

void ShapeGrammar::create_base(SceneModel * building)
{
	//Base is always starting from the origin, and building upwards above the x_z_plane
	//Randomly choose a base type, with constrained random parameters
	int base_type = (int) floor(Util::random(0, 3)) % 3;

	base_type = CYLINDER; //TESTING

	if (base_type == BOX) 
	{
		//Randomize Height and Width
		float width = Util::random(MIN_WIDTH, MAX_WIDTH);
		float height = Util::random(MIN_HEIGHT, MAX_HEIGHT);

		//Make Geometry: Vertices, Normals, Indices, Triangles for Box
		Geometry *box = new Geometry();
		
		glm::vec3 v0 = { width / 2.f, 0, width / 2.f }; //Top Vertices
		glm::vec3 v1 = { -width / 2.f, 0, width / 2.f };
		glm::vec3 v2 = { width / 2.f, 0, -width / 2.f };
		glm::vec3 v3 = { -width / 2.f, 0, -width / 2.f };
		glm::vec3 v4 = { width / 2.f, height, width / 2.f }; //Bottom Vertices
		glm::vec3 v5 = { -width / 2.f, height, width / 2.f };
		glm::vec3 v6 = { width / 2.f, height, -width / 2.f };
		glm::vec3 v7 = { -width / 2.f, height, -width / 2.f };
		glm::vec3 v8 = { DOOR_WIDTH / 2.f, 0, width / 2.f }; //Door Vertices
		glm::vec3 v9 = { -DOOR_WIDTH / 2.f, 0, width / 2.f };
		glm::vec3 v10 = { DOOR_WIDTH / 2.f, DOOR_HEIGHT, width / 2.f };
		glm::vec3 v11 = { -DOOR_WIDTH / 2.f, DOOR_HEIGHT, width / 2.f };
		glm::vec3 v12 = { width / 2.f, DOOR_HEIGHT, width / 2.f }; //Needed for Front Wall w/ Door
		glm::vec3 v13 = { -width / 2.f, DOOR_HEIGHT, width / 2.f };

		//Left Wall
		box->vertices.push_back(v3);
		box->vertices.push_back(v1);
		box->vertices.push_back(v7);
		box->vertices.push_back(v7);
		box->vertices.push_back(v1);
		box->vertices.push_back(v5);		
		for (int i = 0; i < 6; ++i)
			box->normals.push_back(glm::vec3(-1.f, 0.f, 0.f));

		//Right Wall
		box->vertices.push_back(v0);
		box->vertices.push_back(v2);
		box->vertices.push_back(v6);
		box->vertices.push_back(v6);
		box->vertices.push_back(v4);
		box->vertices.push_back(v0);
		for (int i = 0; i < 6; ++i)
			box->normals.push_back(glm::vec3(1.f, 0.f, 0.f));

		//Far Wall
		box->vertices.push_back(v2);
		box->vertices.push_back(v3);
		box->vertices.push_back(v7);
		box->vertices.push_back(v7);
		box->vertices.push_back(v6);
		box->vertices.push_back(v2);
		for (int i = 0; i < 6; ++i)
			box->normals.push_back(glm::vec3(0.f, 0.f, -1.f));

		//Front Wall + Door
		box->vertices.push_back(v1); //Bottom Left
		box->vertices.push_back(v11);
		box->vertices.push_back(v13);
		box->vertices.push_back(v1);
		box->vertices.push_back(v9);
		box->vertices.push_back(v11);
		box->vertices.push_back(v8); //Bottom Right
		box->vertices.push_back(v0); 
		box->vertices.push_back(v10);
		box->vertices.push_back(v10);
		box->vertices.push_back(v0);
		box->vertices.push_back(v12);
		box->vertices.push_back(v13); //Above Door
		box->vertices.push_back(v12);
		box->vertices.push_back(v5);
		box->vertices.push_back(v5);
		box->vertices.push_back(v12);
		box->vertices.push_back(v4);
		for (int i = 0; i < 18; ++i)
			box->normals.push_back(glm::vec3(0.f, 0.f, 1.f));

		for (int i = 0; i < box->vertices.size(); ++i)
			box->indices.push_back(i);
		
		box->populate_buffers();

		Mesh box_mesh = { box, random_material(), ShaderManager::get_default() };
		box_mesh.no_culling = true;
		
		building->add_mesh(box_mesh);

		//Add Next layer
		int num_variations = 5;		
		int next_level = (int) Util::random(0, (float) num_variations) % num_variations;
		if (next_level > 0) next_level++; //Cylinder (1) is not a possibility

		next_level = BOX;

		switch (next_level)
		{
		case BOX:
			add_box(building, height, width);
			break;
		case HEMISPHERE:
			add_hemisphere(building, height, sqrtf(2.f * width * width)); //Width -> Diameter for Hemisphere, so take diagonal width
			break;
		case PYRAMID:
			add_pyramid(building, height, width);
			break;
		case CONE:
			add_cone(building, height, sqrtf(2.f * width * width)); //Width -> Diameter for Cone, so take diagonal width
			break;
		case PLANE:
			add_plane(building, height, width, BOX);
			break;
		default:
			break;
		}
	}
	else if (base_type == CYLINDER)
	{
		//Randomize Height and Width
		float diameter = Util::random(MIN_DIAMETER, MAX_DIAMETER);
		float height = Util::random(MIN_HEIGHT, MAX_HEIGHT);
		float radius = diameter / 2.f;
		//float divisions = ceilf(2.f * glm::pi<float>() * radius / DOOR_WIDTH) * 2.f; //Two Division is the width of the door
		float divisions = (float)NUM_DIVISIONS;
		float width_per_division = 2.f * glm::pi<float>() * radius / divisions;
		float num_divisions_for_door = ceilf(DOOR_WIDTH / width_per_division);
		
		//Make Geometry: Vertices, Normals, Indices, Triangles for Box
		Geometry *cylinder = new Geometry();

		glm::vec3 v_top, v_bot, v0, v1, v2, v3;		

		float step = (2 * glm::pi<float>()) / divisions;

		//Goes Slice by Slice
		for (float i = 0; i < (divisions - num_divisions_for_door); i++)
		{
			float theta = (step * i) + (step * (num_divisions_for_door / 2.f)); //To make door opening, offset start

			v0 = { radius * sinf(theta), height, radius * cosf(theta) };
			v1 = { radius * sinf(theta + step), height, radius * cosf(theta + step) };
			v2 = { radius * sinf(theta), 0, radius * cosf(theta) };
			v3 = { radius * sinf(theta + step), 0, radius * cosf(theta + step) };

			cylinder->vertices.push_back(v0);
			cylinder->vertices.push_back(v1);
			cylinder->vertices.push_back(v2);
			cylinder->vertices.push_back(v1);
			cylinder->vertices.push_back(v3);
			cylinder->vertices.push_back(v2);
			cylinder->normals.push_back(glm::normalize(v0));
			cylinder->normals.push_back(glm::normalize(v1));
			cylinder->normals.push_back(glm::normalize(v2));
			cylinder->normals.push_back(glm::normalize(v1));
			cylinder->normals.push_back(glm::normalize(v3));
			cylinder->normals.push_back(glm::normalize(v2));
		}

		//Indices are just in order to make it easier
		for (int i = 0; i < cylinder->vertices.size(); i++)
			cylinder->indices.push_back(i);

		cylinder->populate_buffers();

		Mesh cylinder_mesh = { cylinder, random_material(), ShaderManager::get_default() };
		cylinder_mesh.no_culling = true;

		building->add_mesh(cylinder_mesh);

		//Add Next layer
		int num_variations = 5;
		int next_level = (int)Util::random(0, (float)num_variations) % num_variations;		
		next_level++; //Startng from Cylinder (1)

		next_level = CYLINDER;

		switch (next_level)
		{
		case CYLINDER:
			add_cylinder(building, height, diameter, num_divisions_for_door / 2.f);
			break;
		case HEMISPHERE:
			add_hemisphere(building, height, diameter, num_divisions_for_door / 2.f);
			break;
		case PYRAMID:
			add_pyramid(building, height, diameter);
			break;
		case CONE:
			add_cone(building, height, diameter, num_divisions_for_door / 2.f);
			break;
		case PLANE:
			add_plane(building, height, diameter, CYLINDER, num_divisions_for_door / 2.f);
			break;
		default:
			break;
		}
	}
	else if (base_type == HEMISPHERE)
	{

	}

	//TODO: Garbage Collector for Created Geometries
}

void ShapeGrammar::add_box(SceneModel * building, float last_height, float last_width)
{
	//Randomize Height and Width
	float width = last_width;
	float height = Util::random(MIN_HEIGHT, MAX_HEIGHT);

	//Make Geometry: Vertices, Normals, Indices, Triangles for Box
	Geometry *box = new Geometry();

	glm::vec3 v0 = { width / 2.f, last_height, width / 2.f }; //Top Vertices
	glm::vec3 v1 = { -width / 2.f, last_height, width / 2.f };
	glm::vec3 v2 = { width / 2.f, last_height, -width / 2.f };
	glm::vec3 v3 = { -width / 2.f, last_height, -width / 2.f };
	glm::vec3 v4 = { width / 2.f, last_height + height, width / 2.f }; //Bottom Vertices
	glm::vec3 v5 = { -width / 2.f, last_height + height, width / 2.f };
	glm::vec3 v6 = { width / 2.f, last_height + height, -width / 2.f };
	glm::vec3 v7 = { -width / 2.f, last_height + height, -width / 2.f };

	//Left Wall
	box->vertices.push_back(v3);
	box->vertices.push_back(v1);
	box->vertices.push_back(v7);
	box->vertices.push_back(v7);
	box->vertices.push_back(v1);
	box->vertices.push_back(v5);
	for (int i = 0; i < 6; ++i)
		box->normals.push_back(glm::vec3(-1.f, 0.f, 0.f));

	//Right Wall
	box->vertices.push_back(v0);
	box->vertices.push_back(v2);
	box->vertices.push_back(v6);
	box->vertices.push_back(v6);
	box->vertices.push_back(v4);
	box->vertices.push_back(v0);
	for (int i = 0; i < 6; ++i)
		box->normals.push_back(glm::vec3(1.f, 0.f, 0.f));

	//Far Wall
	box->vertices.push_back(v2);
	box->vertices.push_back(v3);
	box->vertices.push_back(v7);
	box->vertices.push_back(v7);
	box->vertices.push_back(v6);
	box->vertices.push_back(v2);
	for (int i = 0; i < 6; ++i)
		box->normals.push_back(glm::vec3(0.f, 0.f, -1.f));

	//Front Wall
	box->vertices.push_back(v1);
	box->vertices.push_back(v0);
	box->vertices.push_back(v4);
	box->vertices.push_back(v1);
	box->vertices.push_back(v4);
	box->vertices.push_back(v5);
	for (int i = 0; i < 6; ++i)
		box->normals.push_back(glm::vec3(0.f, 0.f, 1.f));

	for (int i = 0; i < box->vertices.size(); ++i)
		box->indices.push_back(i);

	box->populate_buffers();

	Mesh box_mesh = { box, random_material(), ShaderManager::get_default() };
	box_mesh.no_culling = true;

	building->add_mesh(box_mesh);

	//Add Next layer
	int num_variations = 5;
	int next_level = (int)Util::random(0, (float)num_variations) % num_variations;
	if (next_level > 0) next_level++; //Cylinder (1) is not a possibility

	switch (next_level)
	{
	case BOX:
		add_box(building, last_height + height, width);
		break;
	case HEMISPHERE:
		add_hemisphere(building, last_height + height, sqrtf(2.f * width * width)); //Width -> Diameter for Hemisphere, so take diagonal width
		break;
	case PYRAMID:
		add_pyramid(building, last_height + height, width);
		break;
	case CONE:
		add_cone(building, last_height + height, sqrtf(2.f * width * width)); //Width -> Diameter for Cone, so take diagonal width
		break;
	case PLANE:
		add_plane(building, last_height + height, width, BOX);
		break;
	default:
		break;
	}
}

void ShapeGrammar::add_cylinder(SceneModel * building, float last_height, float last_width, float offset)
{
	//Randomize Height and Width
	float diameter = last_width;
	float height = Util::random(MIN_HEIGHT, MAX_HEIGHT);
	float radius = diameter / 2.f;
	float divisions = (float) NUM_DIVISIONS;

	//Make Geometry: Vertices, Normals, Indices, Triangles for Box
	Geometry *cylinder = new Geometry();

	glm::vec3 v_top, v_bot, v0, v1, v2, v3;

	float step = (2 * glm::pi<float>()) / divisions;

	//Goes Slice by Slice
	for (float i = 0; i < divisions; i++)
	{
		float theta = (step * i) + (step * offset); //To make door opening, offset start

		v0 = { radius * sinf(theta), last_height + height, radius * cosf(theta) };
		v1 = { radius * sinf(theta + step), last_height + height, radius * cosf(theta + step) };
		v2 = { radius * sinf(theta), last_height, radius * cosf(theta) };
		v3 = { radius * sinf(theta + step), last_height, radius * cosf(theta + step) };

		cylinder->vertices.push_back(v0);
		cylinder->vertices.push_back(v1);
		cylinder->vertices.push_back(v2);
		cylinder->vertices.push_back(v1);
		cylinder->vertices.push_back(v3);
		cylinder->vertices.push_back(v2);
		cylinder->normals.push_back(glm::normalize(v0));
		cylinder->normals.push_back(glm::normalize(v1));
		cylinder->normals.push_back(glm::normalize(v2));
		cylinder->normals.push_back(glm::normalize(v1));
		cylinder->normals.push_back(glm::normalize(v3));
		cylinder->normals.push_back(glm::normalize(v2));
	}

	//Indices are just in order to make it easier
	for (int i = 0; i < cylinder->vertices.size(); i++)
		cylinder->indices.push_back(i);

	cylinder->populate_buffers();

	Mesh cylinder_mesh = { cylinder, random_material(), ShaderManager::get_default() };
	cylinder_mesh.no_culling = true;

	building->add_mesh(cylinder_mesh);

	//Add Next layer
	int num_variations = 5;
	int next_level = (int)Util::random(0, (float)num_variations) % num_variations;
	next_level++; //Startng from Cylinder (1)

	switch (next_level)
	{
	case CYLINDER:
		add_cylinder(building, last_height + height, diameter, offset);
		break;
	case HEMISPHERE:
		add_hemisphere(building, last_height + height, diameter, offset);
		break;
	case PYRAMID:
		add_pyramid(building, last_height + height, diameter);
		break;
	case CONE:
		add_cone(building, last_height + height, diameter, offset);
		break;
	case PLANE:
		add_plane(building, last_height + height, diameter, CYLINDER, offset);
		break;
	default:
		break;
	}
}

void ShapeGrammar::add_hemisphere(SceneModel * building, float last_height, float last_width, float offset)
{

}

void ShapeGrammar::add_pyramid(SceneModel * building, float last_height, float last_width)
{

}

void ShapeGrammar::add_cone(SceneModel * building, float last_height, float last_width, float offset)
{

}

void ShapeGrammar::add_plane(SceneModel * building, float last_height, float last_width, int shape, float offset)
{

}

Material ShapeGrammar::random_material()
{
	Material material;

	int num_colors = 5;
	int color = (int) Util::random(0, (float) num_colors) % num_colors;	

	switch (color)
	{
	case 0:
		material.diffuse = material.ambient = color::purple;
		break;
	case 1:
		material.diffuse = material.ambient = color::bone_white;
		break;
	case 2:
		material.diffuse = material.ambient = color::indian_red;
		break;
	case 3:
		material.diffuse = material.ambient = color::autumn_orange;
		break;
	case 4:
		material.diffuse = material.ambient = color::olive_green;
		break;
	default:
		break;
	}
	
	return material;	
}
