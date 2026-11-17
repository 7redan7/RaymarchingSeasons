#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <texture.hpp>

class heightmap
{
private:
	const texture heightTexture;

	const float texelSize;
	const float worldSize;

	glm::ivec2 origin;
	bool initialized;
	int updatedTexels;

	const unsigned int program;

	unsigned int Heightmap_u_RegionOrigin;
	unsigned int Heightmap_u_RegionSize;

	void dispatchRegion(glm::ivec2 regionOrigin, glm::ivec2 regionSize);

public:
	heightmap(int resolution, float texelSize);
	void update(glm::vec2 regionCenter);
	void bindAsSampler(unsigned int unit) const;
	int getResolution() const;
	float getTexelSize() const;
	float getWorldSize() const;
	int getUpdatedTexels() const;
};
