#include <GL/glew.h>

#include <gpuTimer.hpp>

gpuTimer::gpuTimer()
	: current(0), issued(0), hasSample(false), smoothedMs(0.0f)
{
	glGenQueries(frames, queries);
}

void gpuTimer::begin()
{
	if (issued == frames)
	{
		GLint available = 0;
		glGetQueryObjectiv(queries[current], GL_QUERY_RESULT_AVAILABLE, &available);
		if (available)
		{
			GLuint64 elapsed = 0;
			glGetQueryObjectui64v(queries[current], GL_QUERY_RESULT, &elapsed);
			float ms = elapsed * 1e-6f;
			smoothedMs = hasSample ? smoothedMs + smoothing * (ms - smoothedMs) : ms;
			hasSample = true;
		}
	}
	glBeginQuery(GL_TIME_ELAPSED, queries[current]);
}

void gpuTimer::end()
{
	glEndQuery(GL_TIME_ELAPSED);
	current = (current + 1) % frames;
	if (issued < frames)
		++issued;
}

float gpuTimer::getMs() const
{
	return smoothedMs;
}
