#include "Cell.h"

Cell::Cell()
{
	isAir = isCutCell = isTerrain = false;
	terrainPoints.clear();
}
Cell::Cell(int type_CT)
{
	isAir = isCutCell = isTerrain = false;
	terrainPoints.clear();

	if (type_CT == air_CT)
		isAir = true;
	else if (type_CT = terrain_CT)
		isTerrain = true;

}
Cell::Cell( std::vector< Vector3<float> > points)
{
	isTerrain = isAir = isCutCell = true;
	terrainPoints.clear();
	for(int i = 0; i < points.size(); i++)
		terrainPoints.push_back(points[i]);
}
