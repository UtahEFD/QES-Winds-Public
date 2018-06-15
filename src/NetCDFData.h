#pragma once

#include <vector>
#include <netcdf>
#include <stdlib.h>
#include <stdio.h>
#include <string>
using namespace netCDF;
using namespace netCDF::exceptions;

class NetCDFData
{
private:

	float *x, *y, *z;
	int dimX, dimY, dimZ;
	double *u, *v, *w;


public:

	void getData(float* newX, float* newY, float* newZ, double* newU, double* newV, double* newW, int newDX, int newDY, int newDZ);

	bool outputCellFaceResults(std::string fileName);
};