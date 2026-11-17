#include <string>
#include <cstdlib>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <shader.hpp>
#include <heightmap.hpp>

static const int permutation[512] = {
	151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,
	99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,
	11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,
	139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,
	245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,
	200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,
	123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,
	99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,
	11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,
	139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,
	245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,
	200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,
	123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

heightmap::heightmap(int resolution, float texelSize)
	: heightTexture(resolution,
					resolution,
					GL_R32F,
					GL_LINEAR,
					GL_REPEAT),

		texelSize(texelSize),
		worldSize(resolution * texelSize),

		origin(0),
		initialized(false),
		updatedTexels(0),

		program(buildProgram("../src/res/terrain/heightmap.computeshader"))
{
	glUseProgram(program);
	glUniform1iv(glGetUniformLocation(program, "u_Permutation[0]"), 512, permutation);
	glUniform1f(glGetUniformLocation(program, "u_HeightTexelSize"), texelSize);

	Heightmap_u_RegionOrigin = glGetUniformLocation(program, "u_RegionOrigin");
	Heightmap_u_RegionSize = glGetUniformLocation(program, "u_RegionSize");
}


void heightmap::dispatchRegion(glm::ivec2 regionOrigin, glm::ivec2 regionSize)
{
	glUniform2i(Heightmap_u_RegionOrigin, regionOrigin.x, regionOrigin.y);
	glUniform2i(Heightmap_u_RegionSize, regionSize.x, regionSize.y);

	glDispatchCompute((regionSize.x + 15) / 16, (regionSize.y + 15) / 16, 1);
	updatedTexels += regionSize.x * regionSize.y;
}


void heightmap::update(glm::vec2 regionCenter)
{
	int resolution = getResolution();
	glm::ivec2 newOrigin = glm::ivec2(glm::floor(regionCenter / texelSize)) - resolution / 2;
	glm::ivec2 shift = newOrigin - origin;
	updatedTexels = 0;

	if (initialized && shift == glm::ivec2(0))
	{
		return;
	}

	glUseProgram(program);
	heightTexture.bindAsImage(0, GL_WRITE_ONLY);

	if (!initialized || std::abs(shift.x) >= resolution || std::abs(shift.y) >= resolution)
	{
		dispatchRegion(newOrigin, glm::ivec2(resolution));
	}
	else
	{
		if (shift.x > 0)
			dispatchRegion(glm::ivec2(origin.x + resolution, newOrigin.y), glm::ivec2(shift.x, resolution));
		if (shift.x < 0)
			dispatchRegion(newOrigin, glm::ivec2(-shift.x, resolution));
		if (shift.y > 0)
			dispatchRegion(glm::ivec2(newOrigin.x, origin.y + resolution), glm::ivec2(resolution, shift.y));
		if (shift.y < 0)
			dispatchRegion(newOrigin, glm::ivec2(resolution, -shift.y));
	}

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	origin = newOrigin;
	initialized = true;
}

void heightmap::bindAsSampler(unsigned int unit) const
{
	heightTexture.bindAsSampler(unit);
}

int heightmap::getResolution() const
{
	return heightTexture.getResolution().x;
}

float heightmap::getTexelSize() const
{
	return texelSize;
}

float heightmap::getWorldSize() const
{
	return worldSize;
}

int heightmap::getUpdatedTexels() const
{
	return updatedTexels;
}
