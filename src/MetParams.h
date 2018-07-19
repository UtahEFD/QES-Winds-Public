#pragma once

/*
 * This class is a container relating to sensors and metric
 * information read from the xml.
 */

#include "ParseInterface.h"
#include "Sensor.h"

class MetParams : public ParseInterface
{
private:



public:

	bool metInputFlag;
	int numMeasuringSites;
	int maxSizeDataPoints;
	std::string siteName;
	std::string fileName;
	Sensor* sensor;


	virtual void parseValues()
	{
		parsePrimitive<bool>(true, metInputFlag, "metInputFlag");
		parsePrimitive<int>(true, numMeasuringSites, "numMeasuringSites");
		parsePrimitive<int>(true, maxSizeDataPoints, "maxSizeDataPoints");
		parsePrimitive<std::string>(true, siteName, "siteName");
		parsePrimitive<std::string>(true, fileName, "fileName");
		parseElement<Sensor>(true, sensor, "sensor");

	}
};