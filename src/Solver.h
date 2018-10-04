#pragma once

#include "URBInputData.h"
#include "SimulationParameters.h"
#include "Buildings.h"
#include "NonPolyBuilding.h"
#include "RectangularBuilding.h"
#include "Vector3.h"
#include "NetCDFData.h"
#include "Mesh.h"
#include "DTEHeightField.h"
#include "Cell.h"
#include "Sensor.h"
#include <math.h>
#include <vector>

#define MIN_S(x,y) ((x) < (y) ? (x) : (y))
#define MAX_S(x,y) ((x) > (y) ? (x) : (y))

class Solver
{
protected:
	int nx, ny, nz;
	float dx, dy, dz;
    float dxy;
    std::vector<float> dzArray, zm;
    std::vector<float> x,y,z;
	int itermax;

	int num_sites;
	std::vector<int> site_blayer_flag;
	std::vector<float> site_one_overL;
	std::vector<float> site_xcoord;
	std::vector<float> site_ycoord;
	std::vector<float> site_wind_dir;

	std::vector<float> site_z0;
	std::vector<float> site_z_ref;
	std::vector<float> site_U_ref;

    float max_velmag;
    std::vector<Building*> buildings;

    Mesh* mesh;
	Sensor* Sensor1;
    int rooftopFlag;
    int upwindCavityFlag;
    int streetCanyonFlag;
    int streetIntersectionFlag;
    int wakeFlag;
    int sidewallFlag;

    Cell* cells;
    DTEHeightField* DTEHF;
	float z0;

    const int alpha1 = 1;        /// Gaussian precision moduli
    const int alpha2 = 1;        /// Gaussian precision moduli
    const float eta = pow(alpha1/alpha2, 2.0);
    const float A = pow(dx/dy, 2.0);
    const float B = eta*pow(dx/dz, 2.0);
    const float tol = 1e-9;     /// Error tolerance
    const float omega = 1.78;   /// Over-relaxation factor
    const float pi = 4.0f * atan(1.0);

    void printProgress (float percentage);

public:
	Solver(URBInputData* UID, DTEHeightField* DTEHF);

	virtual void solve(NetCDFData* netcdfDat, bool solveWind) = 0;

    void defineWalls(int* iCellFlag, float* n, float* m, float* f, float* e, float* h, float* g);
    void upWind(Building* build, int* iCellFlag, double* u0, double* v0, double* w0, float* z, float* zm);
    void reliefWake(NonPolyBuilding* build, float* u0, float* v0);
};
