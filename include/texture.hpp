#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class texture
{
private:
	const unsigned int id;
	const GLenum internalFormat;
	const glm::ivec2 resolution;

public:
	texture(int width, int height, GLenum internalFormat, GLenum filter, GLenum wrap);
    
	glm::ivec2 getResolution() const;

	void bindAsSampler(unsigned int unit) const;
	void bindAsImage(unsigned int unit, GLenum access) const;
	void attachToFramebuffer(GLenum attachment) const;
};
