#pragma once

#include <GL/glew.h>

class gpuTimer
{
private:
	static constexpr int frames = 4;
	static constexpr float smoothing = 0.05f;

	unsigned int queries[frames];
	int current;
	int issued;
	bool hasSample;
	float smoothedMs;

public:
	gpuTimer();
	void begin();
	void end();
	float getMs() const;
};
