#pragma once
#include <stdint.h>

namespace rng {	
	//Array that represents a random permutation of 0 -> 255
	typedef int permutation256[256];
	
	void createPermutation(permutation256 &p, int seed);
	float randval(int x, int y, const permutation256 &p);
}

namespace perlin {
	//Assume that 0.0 <= x <= 1.0
	float interpolate(float a, float b, float x);
	float noise(float x, float y, const rng::permutation256 &p);
}
