#pragma once

#include "ParseInterface.h"

class C : public ParseInterface
{
public:
	float y;
	int x;

	C()
	{
		y = 0;
		x = 0;
	}

	void parseValues()
	{
		parsePrimative<int>(true, x, "intValx");
		parsePrimative<float>(true, y, "floatValy");
	}
};