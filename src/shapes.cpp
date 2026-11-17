#include "shapes.hpp"

std::vector<glm::vec3> getVerticalSquare(int size, float distance)
{
	std::vector<glm::vec3> vertices;
	float current_x;
	float current_y;

	glm::vec3 current_v0;
	glm::vec3 current_v1;
	glm::vec3 current_v2;
	glm::vec3 current_v3;

	for (int row = 0; row < size; ++row)
	{
		for (int col = 0; col < size; ++col)
		{
			current_x = static_cast<float>(col)*distance;
			current_y = static_cast<float>(row)*distance;

			current_v0 = {current_x           , current_y           , 0.0f};
			current_v1 = {current_x + distance, current_y           , 0.0f};
			current_v2 = {current_x + distance, current_y + distance, 0.0f};
			current_v3 = {current_x           , current_y + distance, 0.0f};

			vertices.push_back(current_v0);
			vertices.push_back(current_v1);
			vertices.push_back(current_v2);

			vertices.push_back(current_v0);
			vertices.push_back(current_v2);
			vertices.push_back(current_v3);
		}
	}

	return vertices;
}

std::vector<glm::vec3> getOrizzontalSquare(int size, float distance)
{
	std::vector<glm::vec3> vertices;
	float current_x;
	float current_z;

	glm::vec3 current_v0;
	glm::vec3 current_v1;
	glm::vec3 current_v2;
	glm::vec3 current_v3;

	for (int row = 0; row < size; ++row)
	{
		for (int col = 0; col < size; ++col)
		{
			current_x = static_cast<float>(col)*distance;
			current_z = -static_cast<float>(row)*distance;

			current_v0 = {current_x,            -5.0f, current_z           };
			current_v1 = {current_x + distance, -5.0f, current_z           };
			current_v2 = {current_x + distance, -5.0f, current_z - distance};
			current_v3 = {current_x,            -5.0f, current_z - distance};

			vertices.push_back(current_v0);
			vertices.push_back(current_v1);
			vertices.push_back(current_v2);

			vertices.push_back(current_v0);
			vertices.push_back(current_v2);
			vertices.push_back(current_v3);
		}
	}

	return vertices;
}

// Per la tessellazione

std::vector<glm::vec3> getOrizzontalSquareV2(int size, float distance)
{
	std::vector<glm::vec3> vertices;
	float current_x;
	float current_z;
	float halfSide = 0.5f * static_cast<float>(size) * distance;

	for (int row = 0; row < size; ++row)
	{
		for (int col = 0; col < size; ++col)
		{
			current_x = static_cast<float>(col)*distance - halfSide;
			current_z = -static_cast<float>(row)*distance + halfSide;

			vertices.push_back({current_x,            -5.0f, current_z           });
			vertices.push_back({current_x + distance, -5.0f, current_z           });
			vertices.push_back({current_x + distance, -5.0f, current_z - distance});
			vertices.push_back({current_x,            -5.0f, current_z - distance});
		}
	}

	return vertices;
}
