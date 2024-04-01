#include "perlin.hpp"
#include <math.h>
#include <glm/glm.hpp>
#include <random>

namespace rng {	
	void createPermutation(permutation256 &p, int seed)
	{
		int values[256];
		int count = 256;
		for(int i = 0; i < 256; i++)
			values[i] = i;

		int index = 0;
		std::minstd_rand lcg;
		lcg.seed(seed);
		while(count > 0) {
			int randindex = lcg() % count;
			p[index++] = values[randindex];
			values[randindex] = values[count - 1];
			count--;
		}
	}
	
	float randval(int x, int y, const permutation256 &p)
	{
		int index = x * 713 + y * 631;
		index %= 256;
		index = labs(index);
		return (float)p[index] / 255.0f;
	}
}

namespace perlin {
	float dotgradient(
		int gridx,
		int gridy,
		float x,
		float y,
		const rng::permutation256 &p
	) {
		float angle = rng::randval(gridx, gridy, p) * 2.0f * M_PI;
		glm::vec2 v(cosf(angle), sinf(angle));
		glm::vec2 d(x - float(gridx), y - float(gridy));
		return glm::dot(v, d);
	}

	float interpolate(float a, float b, float x)
	{
		return (b - a) * (3.0 - x * 2.0) * x * x + a;
	}

	float noise(float x, float y, const rng::permutation256 &p)
	{
		int
			leftx = int(floorf(x)),
			lowery = int(floorf(y)),
			rightx = leftx + 1,
			uppery = lowery + 1;
		float 
			lowerleft = dotgradient(leftx, lowery, x, y, p),
			lowerright = dotgradient(rightx, lowery, x, y, p),
			upperleft = dotgradient(leftx, uppery, x, y, p),
			upperright = dotgradient(rightx, uppery, x, y, p);
		float 
			lerpedlower = interpolate(lowerleft, lowerright, x - leftx),
			lerpedupper = interpolate(upperleft, upperright, x - leftx);
		return interpolate(lerpedlower, lerpedupper, y - lowery);
	}
}
