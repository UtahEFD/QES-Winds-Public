#pragma once

#include "ParseInterface.h"


class Sensor : public ParseInterface
{
private:

public:
		int site_blayer_flag;
		float site_one_overL;
		float site_xcoord;
		float site_ycoord;
		float site_wind_dir;

		float site_z0;
		float site_z_ref;
		float site_U_ref;

		

		virtual void parseValues()
		{
			//rsePrimitive<int>(true, num_sites, "numberofSites");
			//parsePrimitive<int>(true, siteCoords, "siteCoords");
			parsePrimitive<float>(true, site_xcoord, "site_xcoord");
			parsePrimitive<float>(true, site_ycoord, "site_ycoord");
			parsePrimitive<int>(true, site_blayer_flag, "boundaryLayerFlag");
			parsePrimitive<float>(true, site_z0, "siteZ0");
			parsePrimitive<float>(true, site_one_overL, "reciprocal");
			parsePrimitive<float>(true, site_z_ref, "height");
			parsePrimitive<float>(true, site_U_ref, "speed");
			parsePrimitive<float>(true, site_wind_dir, "direction");
	
		}

		/*
		 *This function takes in information for each site' sensor (boundary layer flag, reciprocal coefficient, surface 
		 *roughness and measured wind velocity and direction), generates wind velocity profile for each sensor and finally
	   	 *utilizes Barns scheme to interplote velocity to generate the initial velocity field for the domain.
		 */
	
		void inputWindProfile(float dx, float dy, float dz, int nx, int ny, int nz, double *u0, double *v0, double *w0, 
							 int num_sites, int *site_blayer_flag, float *site_one_overL, float *site_xcoord,
							 float *site_ycoord, float *site_wind_dir, float *site_z0, float *site_z_ref, float *site_U_ref, 
							 float *x, float *y, float *z);

};


