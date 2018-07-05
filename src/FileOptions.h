#pragma once

#include "ParseInterface.h"

class FileOptions : public ParseInterface
{
private:



public:

	int outputFormat;
	bool massConservedFlag;
	bool sensorVelocityFlag;
	bool staggerdVelocityFlag;

	virtual void parseValues()
	{
		parsePrimitive<int>(true, outputFormat, "outputFormat");
		parsePrimitive<bool>(true, massConservedFlag, "massConservedFlag");
		parsePrimitive<bool>(true, sensorVelocityFlag, "sensorVelocityFlag");
		parsePrimitive<bool>(true, staggerdVelocityFlag, "staggerdVelocityFlag");

	}
};