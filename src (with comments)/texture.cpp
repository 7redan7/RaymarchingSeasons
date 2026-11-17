#include <GL/glew.h>

#include <texture.hpp>

static unsigned int generateTextureId()
{
	unsigned int id;
	glGenTextures(1, &id);
	return id;
}

texture::texture(int width, int height, GLenum internalFormat, GLenum filter, GLenum wrap)
	: id(generateTextureId()),
		internalFormat(internalFormat),
		resolution(width, height)
{
	glBindTexture(GL_TEXTURE_2D, id);
	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
}

void texture::bindAsSampler(unsigned int unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void texture::bindAsImage(unsigned int unit, GLenum access) const
{
	glBindImageTexture(unit, id, 0, GL_FALSE, 0, access, internalFormat);
}

void texture::attachToFramebuffer(GLenum attachment) const
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, id, 0);
}

glm::ivec2 texture::getResolution() const
{
	return resolution;
}
