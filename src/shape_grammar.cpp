#include "shape_grammar.h"
#include "util.h"
#include "global.h"

const float PLAYER_HEIGHT = Global::PLAYER_HEIGHT;
const float DOOR_HEIGHT = 25.0f/20.f * PLAYER_HEIGHT; //Should be larger than size of human
const float DOOR_WIDTH = 20.0f/20.f * PLAYER_HEIGHT;
const float MIN_HEIGHT = 27.0f/20.f * PLAYER_HEIGHT; //Random Temporary Values
const float MAX_HEIGHT = 35.0f/20.f * PLAYER_HEIGHT;
const float MIN_WIDTH = 40.0f/20.f * PLAYER_HEIGHT;
const float MAX_WIDTH = 50.0f/20.f * PLAYER_HEIGHT;
const float MIN_DIAMETER = 60.0f/20.f * PLAYER_HEIGHT;
const float MAX_DIAMETER = 70.0f/20.f * PLAYER_HEIGHT;
const int NUM_DIVISIONS = 15;

bool in_shadow;

//Main Generation Function
SceneModel *ShapeGrammar::generate_building(Scene * scene, bool shadowed, int seed)
{
	in_shadow = shadowed;
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
	int base_type = (int) floor(Util::random(0, 2));	

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
		int variations[] = { BOX, PYRAMID, PLANE };
		int next_level = variations[(int) Util::random(0, 3)];

		switch (next_level)
		{
		case BOX:
			add_box(building, height, width);
			break;
		case PYRAMID:
			add_pyramid(building, height, width);
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
		float num_divisions_for_width = ceilf(DOOR_WIDTH / width_per_division);

		//Make Geometry: Vertices, Normals, Indices, Triangles for Box
		Geometry *cylinder = new Geometry();

		glm::vec3 v0, v1, v2, v3;		

		float step = (2 * glm::pi<float>()) / divisions;

		//Goes Slice by Slice
		for (float i = 0; i < divisions; i++)
		{
			if (i >= divisions - num_divisions_for_width) //Make Door Opening
			{
				float theta = (step * i) + (step * (num_divisions_for_width / 2.f));

				v0 = { radius * sinf(theta), height, radius * cosf(theta) };
				v1 = { radius * sinf(theta + step), height, radius * cosf(theta + step) };
				v2 = { radius * sinf(theta), DOOR_HEIGHT, radius * cosf(theta) };
				v3 = { radius * sinf(theta + step), DOOR_HEIGHT, radius * cosf(theta + step) };

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
			else
			{
				float theta = (step * i) + (step * (num_divisions_for_width / 2.f)); //Offset to make door opening easier

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
		}

		//Indices are just in order to make it easier
		for (int i = 0; i < cylinder->vertices.size(); i++)
			cylinder->indices.push_back(i);

		cylinder->populate_buffers();

		Mesh cylinder_mesh = { cylinder, random_material(), ShaderManager::get_default() };
		cylinder_mesh.no_culling = true;

		building->add_mesh(cylinder_mesh);

		//Add Next layer
		int variations[] = { CYLINDER, HEMISPHERE, CONE, PLANE };
		int next_level = variations[(int)Util::random(0, 4)];

		switch (next_level)
		{
		case CYLINDER:
			add_cylinder(building, height, diameter, num_divisions_for_width / 2.f);
			break;
		case HEMISPHERE:
			add_hemisphere(building, height, diameter, num_divisions_for_width / 2.f);
			break;
		case CONE:
			add_cone(building, height, diameter, num_divisions_for_width / 2.f);
			break;
		case PLANE:
			add_plane(building, height, diameter, CYLINDER, num_divisions_for_width / 2.f);
			break;
		default:
			break;
		}
	}
	else if (base_type == HEMISPHERE)
	{
		//Randomize Height and Width
		float diameter = Util::random(MIN_DIAMETER, MAX_DIAMETER);
		float radius = diameter / 2.f;
		//float divisions = ceilf(2.f * glm::pi<float>() * radius / DOOR_WIDTH) * 2.f; //Two Division is the width of the door
		float divisions = (float)NUM_DIVISIONS;
		float width_per_division = 2.f * glm::pi<float>() * radius / divisions;
		float num_divisions_for_width = ceilf(DOOR_WIDTH / width_per_division);
		float num_divisions_for_height = ceilf(DOOR_HEIGHT / width_per_division) * 2;

		//Make Geometry: Vertices, Normals, Indices, Triangles for Box		
		Geometry *hemisphere = new Geometry();

		float fstacks = divisions;
		float fslices = divisions;
		float pi = glm::pi<float>();

		glm::vec3 v0, v1, v2, v3;

		for (float i = (num_divisions_for_width / 2.0f); i < divisions + (num_divisions_for_width / 2.0f); i++)
		{
			if (i >= divisions - (num_divisions_for_width / 2.0f)) //For Door
			{
				for (float j = (divisions / 2) + (num_divisions_for_height); j < divisions; j++)
				{
					//Vertices, like a square
					v0 = { radius * sin(2.0f * pi * i / fstacks) * sin(pi * (j + 1.0f) / fslices),
						radius * -cos(pi * (j + 1.0f) / fslices),
						radius * cos(2.0f * pi * i / fstacks) * sin(pi * (j + 1.0f) / fslices) };

					v1 = { radius * sin(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * j / fslices),
						radius * -cos(pi * j / fslices),
						radius * cos(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * j / fslices) };

					v2 = { radius * sin(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * (j + 1.0) / fslices),
						radius * -cos(pi * (j + 1.0) / fslices),
						radius * cos(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * (j + 1.0) / fslices) };

					v3 = { radius * sin(2.0f * pi * i / fstacks) * sin(pi * j / fslices),
						radius * -cos(pi * j / fslices),
						radius * cos(2.0f * pi * i / fstacks) * sin(pi * j / fslices) };

					//Bottom Left Triangle
					hemisphere->vertices.push_back(v0);
					hemisphere->normals.push_back(glm::normalize(v0));
					hemisphere->vertices.push_back(v1);
					hemisphere->normals.push_back(glm::normalize(v1));
					hemisphere->vertices.push_back(v2);
					hemisphere->normals.push_back(glm::normalize(v2));

					//Bottom Right Triangle
					hemisphere->vertices.push_back(v0);
					hemisphere->normals.push_back(glm::normalize(v0));
					hemisphere->vertices.push_back(v3);
					hemisphere->normals.push_back(glm::normalize(v3));
					hemisphere->vertices.push_back(v1);
					hemisphere->normals.push_back(glm::normalize(v1));
				}
			}
			else //For Everything Else
			{
				for (float j = (divisions / 2); j < divisions; j++)
				{
					//Vertices, like a square
					v0 = { radius * sin(2.0f * pi * i / fstacks) * sin(pi * (j + 1.0f) / fslices),
						radius * -cos(pi * (j + 1.0f) / fslices),
						radius * cos(2.0f * pi * i / fstacks) * sin(pi * (j + 1.0f) / fslices) };

					v1 = { radius * sin(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * j / fslices),
						radius * -cos(pi * j / fslices),
						radius * cos(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * j / fslices) };

					v2 = { radius * sin(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * (j + 1.0) / fslices),
						radius * -cos(pi * (j + 1.0) / fslices),
						radius * cos(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * (j + 1.0) / fslices) };

					v3 = { radius * sin(2.0f * pi * i / fstacks) * sin(pi * j / fslices),
						radius * -cos(pi * j / fslices),
						radius * cos(2.0f * pi * i / fstacks) * sin(pi * j / fslices) };

					//Bottom Left Triangle
					hemisphere->vertices.push_back(v0);
					hemisphere->normals.push_back(glm::normalize(v0));
					hemisphere->vertices.push_back(v1);
					hemisphere->normals.push_back(glm::normalize(v1));
					hemisphere->vertices.push_back(v2);
					hemisphere->normals.push_back(glm::normalize(v2));

					//Bottom Right Triangle
					hemisphere->vertices.push_back(v0);
					hemisphere->normals.push_back(glm::normalize(v0));
					hemisphere->vertices.push_back(v3);
					hemisphere->normals.push_back(glm::normalize(v3));
					hemisphere->vertices.push_back(v1);
					hemisphere->normals.push_back(glm::normalize(v1));
				}
			}			
		}

		for (int i = 0; i < hemisphere->vertices.size(); i++)
			hemisphere->indices.push_back(i);

		hemisphere->populate_buffers();
		Mesh hemisphere_mesh = { hemisphere, random_material(), ShaderManager::get_default() };
		hemisphere_mesh.no_culling = true;

		building->add_mesh(hemisphere_mesh);

		//No More added levels
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
	int variations[] = { BOX, PYRAMID, PLANE };
	int next_level = variations[(int)Util::random(0, 3)];

	switch (next_level)
	{
	case BOX:
		add_box(building, last_height + height, width);
		break;
	case PYRAMID:
		add_pyramid(building, last_height + height, width);
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

	glm::vec3 v0, v1, v2, v3;

	float step = (2 * glm::pi<float>()) / divisions;

	//Goes Slice by Slice
	for (float i = 0; i < divisions; i++)
	{
		float theta = (step * i) + (step * offset);

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
	int variations[] = { CYLINDER, HEMISPHERE, CONE, PLANE };
	int next_level = variations[(int)Util::random(0, 4)];

	switch (next_level)
	{
	case CYLINDER:
		add_cylinder(building, last_height + height, diameter, offset);
		break;
	case HEMISPHERE:
		add_hemisphere(building, last_height + height, diameter, offset);
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
	//Randomize
	float diameter = last_width;
	float radius = diameter / 2.f;
	float divisions = (float)NUM_DIVISIONS;

	//Make Geometry: Vertices, Normals, Indices, Triangles for Box		
	Geometry *hemisphere = new Geometry();

	float fstacks = divisions;
	float fslices = divisions;
	float pi = glm::pi<float>();

	glm::vec3 v0, v1, v2, v3;

	for (float i = offset; i <= divisions; i++)
	{
		for (float j = (divisions / 2); j < divisions; j++)
		{
			//Vertices, like a square
			v0 = { radius * sin(2.0f * pi * i / fstacks) * sin(pi * (j + 1.0f) / fslices),
				radius * -cos(pi * (j + 1.0f) / fslices) + last_height,
				radius * cos(2.0f * pi * i / fstacks) * sin(pi * (j + 1.0f) / fslices) };

			v1 = { radius * sin(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * j / fslices),
				radius * -cos(pi * j / fslices) + last_height,
				radius * cos(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * j / fslices) };

			v2 = { radius * sin(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * (j + 1.0) / fslices),
				radius * -cos(pi * (j + 1.0) / fslices) + last_height,
				radius * cos(2.0f * pi * (i + 1.0) / fstacks) * sin(pi * (j + 1.0) / fslices) };

			v3 = { radius * sin(2.0f * pi * i / fstacks) * sin(pi * j / fslices),
				radius * -cos(pi * j / fslices) + last_height,
				radius * cos(2.0f * pi * i / fstacks) * sin(pi * j / fslices) };

			//Bottom Left Triangle
			hemisphere->vertices.push_back(v0);
			hemisphere->normals.push_back(glm::normalize(v0));
			hemisphere->vertices.push_back(v1);
			hemisphere->normals.push_back(glm::normalize(v1));
			hemisphere->vertices.push_back(v2);
			hemisphere->normals.push_back(glm::normalize(v2));

			//Bottom Right Triangle
			hemisphere->vertices.push_back(v0);
			hemisphere->normals.push_back(glm::normalize(v0));
			hemisphere->vertices.push_back(v3);
			hemisphere->normals.push_back(glm::normalize(v3));
			hemisphere->vertices.push_back(v1);
			hemisphere->normals.push_back(glm::normalize(v1));
		}
	}

	for (int i = 0; i < hemisphere->vertices.size(); i++)
		hemisphere->indices.push_back(i);

	hemisphere->populate_buffers();
	Mesh hemisphere_mesh = { hemisphere, random_material(), ShaderManager::get_default() };
	hemisphere_mesh.no_culling = true;

	building->add_mesh(hemisphere_mesh);
}

void ShapeGrammar::add_pyramid(SceneModel * building, float last_height, float last_width)
{
	//Randomize Height and Width
	float width = last_width;
	float height = Util::random(MIN_HEIGHT, MAX_HEIGHT);

	//Make Geometry: Vertices, Normals, Indices, Triangles for Box
	Geometry *pyramid = new Geometry();

	glm::vec3 v0 = { width / 2.f, last_height, width / 2.f }; //Bottom Vertices
	glm::vec3 v1 = { -width / 2.f, last_height, width / 2.f };
	glm::vec3 v2 = { width / 2.f, last_height, -width / 2.f };
	glm::vec3 v3 = { -width / 2.f, last_height, -width / 2.f };

	//Randomize Top Point Location (To add more roof variety. If not at center, it becomes a slanted roof)
	glm::vec2 top_point_loc[] = { {0.f, 0.f}, {width / 2.f, width / 2.f}, {-width / 2.f, width / 2.f}, {width / 2.f, -width / 2.f} };
	int loc = (int)Util::random(0, 2);
	glm::vec3 v4 = { top_point_loc[loc].x, last_height + height, top_point_loc[loc].y }; //Top Point

	//Left Wall
	pyramid->vertices.push_back(v3);
	pyramid->vertices.push_back(v1);
	pyramid->vertices.push_back(v4);	
	for (int i = 0; i < 3; ++i)
		pyramid->normals.push_back(glm::vec3(-width / 4.f, height / 2.f, 0.f));

	//Right Wall
	pyramid->vertices.push_back(v4);
	pyramid->vertices.push_back(v0);
	pyramid->vertices.push_back(v2);
	for (int i = 0; i < 3; ++i)
		pyramid->normals.push_back(glm::vec3(width / 4.f, height / 2.f, 0.f));

	//Far Wall
	pyramid->vertices.push_back(v4);
	pyramid->vertices.push_back(v2);
	pyramid->vertices.push_back(v3);
	for (int i = 0; i < 3; ++i)
		pyramid->normals.push_back(glm::vec3(0.f, height / 2.f, -width / 4.f));

	//Front Wall
	pyramid->vertices.push_back(v1);
	pyramid->vertices.push_back(v0);
	pyramid->vertices.push_back(v4);
	for (int i = 0; i < 3; ++i)
		pyramid->normals.push_back(glm::vec3(0.f, height / 2.f, width / 4.f));

	for (int i = 0; i < pyramid->vertices.size(); ++i)
		pyramid->indices.push_back(i);

	pyramid->populate_buffers();

	Mesh pyramid_mesh = { pyramid, random_material(), ShaderManager::get_default() };
	pyramid_mesh.no_culling = true;

	building->add_mesh(pyramid_mesh);
}

void ShapeGrammar::add_cone(SceneModel * building, float last_height, float last_width, float offset)
{
	//Randomize Height and Width
	float diameter = last_width;
	float height = Util::random(MIN_HEIGHT, MAX_HEIGHT);
	float radius = diameter / 2.f;
	float divisions = (float)NUM_DIVISIONS;

	//Make Geometry: Vertices, Normals, Indices, Triangles for Box
	Geometry *cone = new Geometry();

	glm::vec3 v0, v1;
	glm::vec3 v_top = { 0, last_height + height, 0 };

	float step = (2 * glm::pi<float>()) / divisions;

	//Goes Slice by Slice
	for (float i = 0; i < divisions; i++)
	{
		float theta = (step * i) + (step * offset); 

		v0 = { radius * sinf(theta), last_height, radius * cosf(theta) };
		v1 = { radius * sinf(theta + step), last_height, radius * cosf(theta + step) };

		cone->vertices.push_back(v0);
		cone->vertices.push_back(v1);
		cone->vertices.push_back(v_top);

		cone->normals.push_back(glm::normalize(v0));
		cone->normals.push_back(glm::normalize(v1));
		cone->normals.push_back(glm::normalize(v_top));
	}

	//Indices are just in order to make it easier
	for (int i = 0; i < cone->vertices.size(); i++)
		cone->indices.push_back(i);

	cone->populate_buffers();

	Mesh cone_mesh = { cone, random_material(), ShaderManager::get_default() };
	cone_mesh.no_culling = true;

	building->add_mesh(cone_mesh);
}

void ShapeGrammar::add_plane(SceneModel * building, float last_height, float last_width, int shape, float offset)
{
	//Closes Previous Shape

	if (shape == BOX)
	{		
		float width = last_width;

		//Make Geometry: Vertices, Normals, Indices, Triangles for Box
		Geometry *plane = new Geometry();

		glm::vec3 v0 = { width / 2.f, last_height, width / 2.f };
		glm::vec3 v1 = { -width / 2.f, last_height, width / 2.f };
		glm::vec3 v2 = { width / 2.f, last_height, -width / 2.f };
		glm::vec3 v3 = { -width / 2.f, last_height, -width / 2.f };		

		plane->vertices.push_back(v1);
		plane->vertices.push_back(v0);
		plane->vertices.push_back(v2);
		plane->vertices.push_back(v2);
		plane->vertices.push_back(v3);
		plane->vertices.push_back(v1);
		for (int i = 0; i < 6; ++i)
			plane->normals.push_back(glm::vec3(0.f, 1.f, 0.f));

		for (int i = 0; i < plane->vertices.size(); ++i)
			plane->indices.push_back(i);

		plane->populate_buffers();

		Mesh plane_mesh = { plane, random_material(), ShaderManager::get_default() };
		plane_mesh.no_culling = true;

		building->add_mesh(plane_mesh);
	}
	else if (shape == CYLINDER)
	{
		//Randomize Height and Width
		float diameter = last_width;
		float radius = diameter / 2.f;
		float divisions = (float)NUM_DIVISIONS;

		//Make Geometry: Vertices, Normals, Indices, Triangles for Box
		Geometry *cylinder = new Geometry();

		glm::vec3 v0, v1;
		glm::vec3 v_mid = { 0, last_height, 0 };

		float step = (2 * glm::pi<float>()) / divisions;

		for (float i = 0; i < divisions; i++)
		{
			float theta = (step * i) + (step * offset);

			v0 = { radius * sinf(theta), last_height, radius * cosf(theta) };
			v1 = { radius * sinf(theta + step), last_height, radius * cosf(theta + step) };

			cylinder->vertices.push_back(v0);
			cylinder->vertices.push_back(v1);
			cylinder->vertices.push_back(v_mid);
			for (int i = 0; i < 3; ++i)
				cylinder->normals.push_back(glm::vec3(0.f, 1.f, 0.f));
		}

		//Indices are just in order to make it easier
		for (int i = 0; i < cylinder->vertices.size(); i++)
			cylinder->indices.push_back(i);

		cylinder->populate_buffers();

		Mesh cylinder_mesh = { cylinder, random_material(), ShaderManager::get_default() };
		cylinder_mesh.no_culling = true;

		building->add_mesh(cylinder_mesh);
	}
}

Material ShapeGrammar::random_material()
{
	Material material;

	glm::vec3 leaf_colors[] = { color::olive_green, color::autumn_orange, color::purple, color::wood_tan_light, color::indian_red };
	material.diffuse = material.ambient = leaf_colors[(int)Util::random(0, 5)];
	material.shadows = in_shadow;

	return material;	
}
