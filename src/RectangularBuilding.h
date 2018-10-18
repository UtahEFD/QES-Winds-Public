#pragma once

#include "ParseInterface.h"
#include "NonPolyBuilding.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using std::cerr;
using std::endl;
using std::vector;
using std::cout;


class RectangularBuilding : public NonPolyBuilding
{
private:

	int icell_cent, icell_cut;
	float H;


public:

	RectangularBuilding()
	{
		buildingGeometry = 1;

	}

	RectangularBuilding(float xfo, float yfo, float bh, float dx, float dy, float dz)
	{
		buildingGeometry = 1;

		groupID = 999;
		buildingType = 5;
		H = dz;
		baseHeight = bh;
		x_start = xfo;
		y_start = yfo;
		L = dx;
		W = dy;

	}

	virtual void parseValues()
	{
		parsePrimitive<int>(true, groupID, "groupID");
		parsePrimitive<int>(true, buildingType, "buildingType");
		parsePrimitive<float>(true, H, "height");
		parsePrimitive<float>(true, baseHeight, "baseHeight");
		parsePrimitive<float>(true, x_start, "xFo");
		parsePrimitive<float>(true, y_start, "yFo");
		parsePrimitive<float>(true, L, "length");
		parsePrimitive<float>(true, W, "width");

	}

	void setBoundaries(float dx, float dy, float dz, int nx, int ny, int nz, float *zm, float *e, float *f, float *g, float *h, float *m, float *n, int *icellflag, std::vector<std::vector<std::vector<float>>> &x_cut, std::vector<std::vector<std::vector<float>>> &y_cut, std::vector<std::vector<std::vector<float>>> &z_cut, std::vector<std::vector<int>> &num_points, std::vector<std::vector<float>> &coeff)
	{


		/// intersection points hard-coded	
		for (int j=j_cut_start+1; j<j_cut_end; j++){
			for (int k=1; k<k_cut_end; k++){
				icell_cut = i_cut_start + j*(nx-1) + k*(nx-1)*(ny-1);
				if (icellflag[icell_cut]==7){
					num_points[icell_cut][0] = num_points[icell_cut][2] = num_points[icell_cut][3] = num_points[icell_cut][4] = num_points[icell_cut][5] = 4;
					num_points[icell_cut][1] = 0;

					y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = dy;
					y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = 0.0;
					z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = 0.0;
					z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = dz;

					x_cut[icell_cut][2][0] = x_cut[icell_cut][2][3] = x_cut[icell_cut][3][0] = x_cut[icell_cut][3][3] = 0.0;
					x_cut[icell_cut][2][1] = x_cut[icell_cut][2][2] = x_cut[icell_cut][3][1] = x_cut[icell_cut][3][2] = x_start-i_cut_start*dx;
					z_cut[icell_cut][2][0] = z_cut[icell_cut][2][1] = z_cut[icell_cut][3][0] = z_cut[icell_cut][3][1] = 0.0;
					z_cut[icell_cut][2][2] = z_cut[icell_cut][2][3] = z_cut[icell_cut][3][2] = z_cut[icell_cut][3][3] = dz;

					x_cut[icell_cut][4][0] = x_cut[icell_cut][4][3] = x_cut[icell_cut][5][0] = x_cut[icell_cut][5][3] = 0.0;
					x_cut[icell_cut][4][1] = x_cut[icell_cut][4][2] = x_cut[icell_cut][5][1] = x_cut[icell_cut][5][2] = x_start-i_cut_start*dx;
					y_cut[icell_cut][4][0] = y_cut[icell_cut][4][1] = y_cut[icell_cut][5][0] = y_cut[icell_cut][5][1] = 0.0;
					y_cut[icell_cut][4][2] = y_cut[icell_cut][4][3] = y_cut[icell_cut][5][2] = y_cut[icell_cut][5][3] = dy;
				
				}
				icell_cut = (i_cut_end-1) + j*(nx-1) + k*(nx-1)*(ny-1);
				if (icellflag[icell_cut]==7){
					num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][3] = num_points[icell_cut][4] = num_points[icell_cut][5] = 4;
					num_points[icell_cut][0] = 0;

					y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = dy;
					y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
					z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
					z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;
	
					x_cut[icell_cut][2][2] = x_cut[icell_cut][2][3] = x_cut[icell_cut][3][2] = x_cut[icell_cut][3][3] = dx;
					x_cut[icell_cut][2][1] = x_cut[icell_cut][2][0] = x_cut[icell_cut][3][1] = x_cut[icell_cut][3][0] = x_start+L-i_end*dx;
					z_cut[icell_cut][2][2] = z_cut[icell_cut][2][1] = z_cut[icell_cut][3][2] = z_cut[icell_cut][3][1] = 0.0;
					z_cut[icell_cut][2][0] = z_cut[icell_cut][2][3] = z_cut[icell_cut][3][0] = z_cut[icell_cut][3][3] = dz;
	
					x_cut[icell_cut][4][2] = x_cut[icell_cut][4][3] = x_cut[icell_cut][5][2] = x_cut[icell_cut][5][3] = dx;
					x_cut[icell_cut][4][1] = x_cut[icell_cut][4][0] = x_cut[icell_cut][5][1] = x_cut[icell_cut][5][0] = x_start+L-i_end*dx;
					y_cut[icell_cut][4][2] = y_cut[icell_cut][4][1] = y_cut[icell_cut][5][2] = y_cut[icell_cut][5][1] = 0.0;
					y_cut[icell_cut][4][0] = y_cut[icell_cut][4][3] = y_cut[icell_cut][5][0] = y_cut[icell_cut][5][3] = dy;
				}
			}
		}

		for (int i=i_cut_start+1; i<i_cut_end; i++){
			for (int k=1; k<k_cut_end; k++){
				icell_cut = i + j_cut_start*(nx-1) + k*(nx-1)*(ny-1);
				if (icellflag[icell_cut]==7){
					num_points[icell_cut][0] = num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][4] = num_points[icell_cut][5] = 4;
					num_points[icell_cut][3] = 0;
	
					x_cut[icell_cut][2][0] = x_cut[icell_cut][2][3] = 0.0;
					x_cut[icell_cut][2][1] = x_cut[icell_cut][2][2] = dx;
					z_cut[icell_cut][2][0] = z_cut[icell_cut][2][1] = 0.0;
					z_cut[icell_cut][2][2] = z_cut[icell_cut][2][3] = dz;
	
					y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = y_start-j_cut_start*dy;
					y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
					z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
					z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;
	
					x_cut[icell_cut][4][0] = x_cut[icell_cut][4][3] = x_cut[icell_cut][5][0] = x_cut[icell_cut][5][3] = 0.0;
					x_cut[icell_cut][4][1] = x_cut[icell_cut][4][2] = x_cut[icell_cut][5][1] = x_cut[icell_cut][5][2] = dx;
					y_cut[icell_cut][4][0] = y_cut[icell_cut][4][1] = y_cut[icell_cut][5][0] = y_cut[icell_cut][5][1] = 0.0;
					y_cut[icell_cut][4][2] = y_cut[icell_cut][4][3] = y_cut[icell_cut][5][2] = y_cut[icell_cut][5][3] = y_start-j_cut_start*dy;
				}
				icell_cut = i + (j_cut_end-1)*(nx-1) + k*(nx-1)*(ny-1);
				if (icellflag[icell_cut]==7){
					num_points[icell_cut][0] = num_points[icell_cut][1] = num_points[icell_cut][3] = num_points[icell_cut][4] = num_points[icell_cut][5] = 4;
					num_points[icell_cut][2] = 0;
	
					x_cut[icell_cut][3][0] = x_cut[icell_cut][3][3] = 0.0;
					x_cut[icell_cut][3][1] = x_cut[icell_cut][3][2] = dx;
					z_cut[icell_cut][3][0] = z_cut[icell_cut][3][1] = 0.0;
					z_cut[icell_cut][3][2] = z_cut[icell_cut][3][3] = dz;
	
					y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = dy;
					y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = y_start+W-j_end*dy;
					z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
					z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;
	
					x_cut[icell_cut][4][0] = x_cut[icell_cut][4][3] = x_cut[icell_cut][5][1] = x_cut[icell_cut][5][0] = 0.0;
					x_cut[icell_cut][4][1] = x_cut[icell_cut][4][2] = x_cut[icell_cut][5][2] = x_cut[icell_cut][5][3] = dx;
					y_cut[icell_cut][4][0] = y_cut[icell_cut][4][1] = y_cut[icell_cut][5][1] = y_cut[icell_cut][5][2] = y_start+W-j_end*dy;
					y_cut[icell_cut][4][2] = y_cut[icell_cut][4][3] = y_cut[icell_cut][5][3] = y_cut[icell_cut][5][0] = dy;
				}
			}
		}
		
		for (int i=i_cut_start+1; i<i_cut_end; i++){
			for (int j=j_cut_start+1; j<j_cut_end; j++){
				icell_cut = i + j*(nx-1) + k_end*(nx-1)*(ny-1);
				if (icellflag[icell_cut]==7){
					num_points[icell_cut][0] = num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][3] = num_points[icell_cut][5] = 4;
					num_points[icell_cut][4] = 0;

					x_cut[icell_cut][5][0] = x_cut[icell_cut][5][3] = 0.0;
					x_cut[icell_cut][5][1] = x_cut[icell_cut][5][2] = dx;
					y_cut[icell_cut][5][0] = y_cut[icell_cut][5][1] = 0.0;
					y_cut[icell_cut][5][2] = y_cut[icell_cut][5][3] = dy;
	
					y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = dy;
					y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
					z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = H-(k_end-1)*dz;
					z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;
	
					x_cut[icell_cut][2][0] = x_cut[icell_cut][2][3] = x_cut[icell_cut][3][0] = x_cut[icell_cut][3][3] = 0.0;
					x_cut[icell_cut][2][1] = x_cut[icell_cut][2][2] = x_cut[icell_cut][3][1] = x_cut[icell_cut][3][2] = dx;
					z_cut[icell_cut][2][0] = z_cut[icell_cut][2][1] = z_cut[icell_cut][3][0] = z_cut[icell_cut][3][1] = H-(k_end-1)*dz;
					z_cut[icell_cut][2][2] = z_cut[icell_cut][2][3] = z_cut[icell_cut][3][2] = z_cut[icell_cut][3][3] = dz;
				}
			}
		}	
	
		for (int k=1; k<k_end; k++){		
			icell_cut = i_cut_start + j_cut_start*(nx-1) + k*(nx-1)*(ny-1);
			if (icellflag[icell_cut]==7){
				num_points[icell_cut][0] = num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][3] = 4;
				num_points[icell_cut][5] = num_points[icell_cut][4] = 6;

				y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = dy;
				y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = 0.0;
				z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = 0.0;
				z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = dz;

				y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = y_start-j_cut_start*dy;
				y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
				z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
				z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;

				x_cut[icell_cut][2][0] = x_cut[icell_cut][2][3] = 0.0;
				x_cut[icell_cut][2][1] = x_cut[icell_cut][2][2] = dx;
				z_cut[icell_cut][2][0] = z_cut[icell_cut][2][1] = 0.0;
				z_cut[icell_cut][2][2] = z_cut[icell_cut][2][3] = dz;
			
				x_cut[icell_cut][3][2] = x_cut[icell_cut][3][3] = x_start-i_cut_start*dx;
				x_cut[icell_cut][3][0] = x_cut[icell_cut][3][1] = 0.0;
				z_cut[icell_cut][3][0] = z_cut[icell_cut][3][3] = dz;
				z_cut[icell_cut][3][2] = z_cut[icell_cut][3][1] = 0.0;

				x_cut[icell_cut][4][0] = x_cut[icell_cut][4][1] = 0.0;
				x_cut[icell_cut][4][2] = x_cut[icell_cut][4][3] = dx;
				x_cut[icell_cut][4][4] = x_cut[icell_cut][4][5] = x_start-i_cut_start*dx;
				y_cut[icell_cut][4][1] = y_cut[icell_cut][4][2] = 0.0;
				y_cut[icell_cut][4][0] = y_cut[icell_cut][4][5] = dy;
				y_cut[icell_cut][4][4] = y_cut[icell_cut][4][3] = y_start-j_cut_start*dy;
	
				x_cut[icell_cut][5][0] = x_cut[icell_cut][5][1] = 0.0;
				x_cut[icell_cut][5][2] = x_cut[icell_cut][5][3] = dx;
				x_cut[icell_cut][5][4] = x_cut[icell_cut][5][5] = x_start-i_cut_start*dx;
				y_cut[icell_cut][5][1] = y_cut[icell_cut][5][2] = 0.0;
				y_cut[icell_cut][5][3] = y_cut[icell_cut][5][4] = y_start-j_cut_start*dy;
				y_cut[icell_cut][5][0] = y_cut[icell_cut][5][5] = dy;
		
			}
		}

		for (int k=1; k<k_end; k++){		
			icell_cut = (i_cut_end-1) + j_cut_start*(nx-1) + k*(nx-1)*(ny-1);
			if (icellflag[icell_cut]==7){

				num_points[icell_cut][0] = num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][3] = 4;
				num_points[icell_cut][5] = num_points[icell_cut][4] = 6;
	
				y_cut[icell_cut][0][0] = y_cut[icell_cut][0][1] = 0.0;
				y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = y_start-j_cut_start*dy;
				z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = dz;
				z_cut[icell_cut][0][1] = z_cut[icell_cut][0][2] = 0.0;

				y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = dy;
				y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
				z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
				z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;
				
				x_cut[icell_cut][2][0] = x_cut[icell_cut][2][1] = 0.0;
				x_cut[icell_cut][2][2] = x_cut[icell_cut][2][3] = dx;
				z_cut[icell_cut][2][1] = z_cut[icell_cut][2][2] = 0.0;
				z_cut[icell_cut][2][0] = z_cut[icell_cut][2][3] = dz;

				x_cut[icell_cut][3][0] = x_cut[icell_cut][3][1] = x_start+L-i_end*dx;
				x_cut[icell_cut][3][2] = x_cut[icell_cut][3][3] = dx;
				z_cut[icell_cut][3][1] = z_cut[icell_cut][3][2] = 0.0;
				z_cut[icell_cut][3][0] = z_cut[icell_cut][3][3] = dz;

				y_cut[icell_cut][4][4] = y_cut[icell_cut][4][3] = dy;
				y_cut[icell_cut][4][1] = y_cut[icell_cut][4][2] = 0.0;
				y_cut[icell_cut][4][0] = y_cut[icell_cut][4][5] = y_start-j_cut_start*dy;
				x_cut[icell_cut][4][0] = x_cut[icell_cut][4][1] = 0.0;
				x_cut[icell_cut][4][2] = x_cut[icell_cut][4][3] = dx;
				x_cut[icell_cut][4][4] = x_cut[icell_cut][4][5] = x_start+L-i_end*dx;
	
				y_cut[icell_cut][5][4] = y_cut[icell_cut][5][3] = dy;
				y_cut[icell_cut][5][1] = y_cut[icell_cut][5][2] = 0.0;
				y_cut[icell_cut][5][0] = y_cut[icell_cut][5][5] = y_start-j_cut_start*dy;
				x_cut[icell_cut][5][0] = x_cut[icell_cut][5][1] = 0.0;
				x_cut[icell_cut][5][2] = x_cut[icell_cut][5][3] = dx;
				x_cut[icell_cut][5][4] = x_cut[icell_cut][5][5] = x_start+L-i_end*dx;

			}
		}

		for (int k=1; k<k_end; k++){		
			icell_cut = i_cut_start + (j_cut_end-1)*(nx-1) + k*(nx-1)*(ny-1);
			if (icellflag[icell_cut]==7){


				num_points[icell_cut][0] = num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][3] = 4;
				num_points[icell_cut][5] = num_points[icell_cut][4] = 6;

				y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = dy;
				y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = 0.0;
				z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = 0.0;
				z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = dz;

				y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = dy;
				y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = y_start+W-j_end*dy;
				z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
				z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;

				x_cut[icell_cut][2][2] = x_cut[icell_cut][2][3] = x_start-i_cut_start*dx;
				x_cut[icell_cut][2][0] = x_cut[icell_cut][2][1] = 0.0;
				z_cut[icell_cut][2][0] = z_cut[icell_cut][2][3] = dz;
				z_cut[icell_cut][2][2] = z_cut[icell_cut][2][1] = 0.0;

				x_cut[icell_cut][3][0] = x_cut[icell_cut][3][3] = 0.0;
				x_cut[icell_cut][3][1] = x_cut[icell_cut][3][2] = dx;
				z_cut[icell_cut][3][0] = z_cut[icell_cut][3][1] = 0.0;
				z_cut[icell_cut][3][2] = z_cut[icell_cut][3][3] = dz;

				x_cut[icell_cut][4][0] = x_cut[icell_cut][4][1] = 0.0;
				x_cut[icell_cut][4][2] = x_cut[icell_cut][4][3] = x_start-i_cut_start*dx;
				x_cut[icell_cut][4][4] = x_cut[icell_cut][4][5] = dx;
				y_cut[icell_cut][4][1] = y_cut[icell_cut][4][2] = 0.0;
				y_cut[icell_cut][4][0] = y_cut[icell_cut][4][5] = dy;
				y_cut[icell_cut][4][4] = y_cut[icell_cut][4][3] = y_start+W-j_end*dy;
	
				x_cut[icell_cut][5][0] = x_cut[icell_cut][5][1] = 0.0;
				x_cut[icell_cut][5][2] = x_cut[icell_cut][5][3] = x_start-i_cut_start*dx;
				x_cut[icell_cut][5][4] = x_cut[icell_cut][5][5] = dx;
				y_cut[icell_cut][5][1] = y_cut[icell_cut][5][2] = 0.0;
				y_cut[icell_cut][5][0] = y_cut[icell_cut][5][5] = dy;
				y_cut[icell_cut][5][4] = y_cut[icell_cut][5][3] = y_start+W-j_end*dy;

			}
		}

		for (int k=1; k<k_end; k++){		
			icell_cut = (i_cut_end-1) + (j_cut_end-1)*(nx-1) + k*(nx-1)*(ny-1);
			if (icellflag[icell_cut]==7){


				num_points[icell_cut][0] = num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][3] = 4;
				num_points[icell_cut][5] = num_points[icell_cut][4] = 6;
	
				y_cut[icell_cut][0][0] = y_cut[icell_cut][0][1] = y_start+W-j_end*dy;
				y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = dy;
				z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = dz;
				z_cut[icell_cut][0][1] = z_cut[icell_cut][0][2] = 0.0;

				y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = dy;
				y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
				z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
				z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;
				
				x_cut[icell_cut][2][0] = x_cut[icell_cut][2][1] = x_start+L-i_end*dx;
				x_cut[icell_cut][2][2] = x_cut[icell_cut][2][3] = dx;
				z_cut[icell_cut][2][1] = z_cut[icell_cut][2][2] = 0.0;
				z_cut[icell_cut][2][0] = z_cut[icell_cut][2][3] = dz;

				x_cut[icell_cut][3][0] = x_cut[icell_cut][3][1] = 0.0;
				x_cut[icell_cut][3][2] = x_cut[icell_cut][3][3] = dx;
				z_cut[icell_cut][3][1] = z_cut[icell_cut][3][2] = 0.0;
				z_cut[icell_cut][3][0] = z_cut[icell_cut][3][3] = dz;

				y_cut[icell_cut][4][3] = y_cut[icell_cut][4][4] = 0.0;
				y_cut[icell_cut][4][0] = y_cut[icell_cut][4][5] = dy;
				y_cut[icell_cut][4][1] = y_cut[icell_cut][4][2] = y_start+W-j_end*dy;
				x_cut[icell_cut][4][0] = x_cut[icell_cut][4][1] = 0.0;
				x_cut[icell_cut][4][2] = x_cut[icell_cut][4][3] = x_start+L-i_end*dx;
				x_cut[icell_cut][4][4] = x_cut[icell_cut][4][5] = dx;
	
				y_cut[icell_cut][5][3] = y_cut[icell_cut][5][4] = 0.0;
				y_cut[icell_cut][5][0] = y_cut[icell_cut][5][5] = dy;
				y_cut[icell_cut][5][1] = y_cut[icell_cut][5][2] = y_start+W-j_end*dy;
				x_cut[icell_cut][5][0] = x_cut[icell_cut][5][1] = 0.0;
				x_cut[icell_cut][5][2] = x_cut[icell_cut][5][3] = x_start+L-i_end*dx;
				x_cut[icell_cut][5][4] = x_cut[icell_cut][5][5] = dx;
			}
		}

			icell_cut = i_cut_start + j_cut_start*(nx-1) + k_end*(nx-1)*(ny-1);
			if (icellflag[icell_cut]==7){
				num_points[icell_cut][0] = num_points[icell_cut][2] = num_points[icell_cut][5] = 4;
				num_points[icell_cut][1] = num_points[icell_cut][3] = num_points[icell_cut][4] = 6;

				y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = dy;
				y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = 0.0;
				z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = 0.0;
				z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = dz;

				x_cut[icell_cut][2][0] = x_cut[icell_cut][2][3] = 0.0;
				x_cut[icell_cut][2][1] = x_cut[icell_cut][2][2] = dx;
				z_cut[icell_cut][2][0] = z_cut[icell_cut][2][1] = 0.0;
				z_cut[icell_cut][2][2] = z_cut[icell_cut][2][3] = dz;
			
				x_cut[icell_cut][5][0] = x_cut[icell_cut][5][3] = 0.0;
				x_cut[icell_cut][5][1] = x_cut[icell_cut][5][2] = dx;
				y_cut[icell_cut][5][0] = y_cut[icell_cut][5][1] = 0.0;
				y_cut[icell_cut][5][2] = y_cut[icell_cut][5][3] = dy;
	
				y_cut[icell_cut][1][4] = y_cut[icell_cut][1][5] = dy;
				y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
				y_cut[icell_cut][1][3] = y_cut[icell_cut][1][2] = y_start-j_cut_start*dy;
				z_cut[icell_cut][1][4] = z_cut[icell_cut][1][3] = H-(k_end-1)*dz;
				z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
				z_cut[icell_cut][1][0] = z_cut[icell_cut][1][5] = dz;
		
				x_cut[icell_cut][3][0] = x_cut[icell_cut][3][5] = 0.0;
				x_cut[icell_cut][3][3] = x_cut[icell_cut][3][4] = dx;
				x_cut[icell_cut][3][1] = x_cut[icell_cut][3][2] = x_start-i_cut_start*dx;
				z_cut[icell_cut][3][0] = z_cut[icell_cut][3][1] = 0.0;
				z_cut[icell_cut][3][2] = z_cut[icell_cut][3][3] = H-(k_end-1)*dz;
				z_cut[icell_cut][3][4] = z_cut[icell_cut][3][5] = dz;

				x_cut[icell_cut][4][0] = x_cut[icell_cut][4][5] = 0.0;
				x_cut[icell_cut][4][1] = x_cut[icell_cut][4][2] = dx;
				x_cut[icell_cut][4][3] = x_cut[icell_cut][4][4] = x_start-i_cut_start*dx;
				y_cut[icell_cut][4][0] = y_cut[icell_cut][4][1] = 0.0;
				y_cut[icell_cut][4][2] = y_cut[icell_cut][4][3] = y_start-j_cut_start*dy;
				y_cut[icell_cut][4][4] = y_cut[icell_cut][4][5] = dy;
			}

			icell_cut = (i_cut_end-1) + j_cut_start*(nx-1) + k_end*(nx-1)*(ny-1);
			if (icellflag[icell_cut]==7){
				num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][5] = 4;
				num_points[icell_cut][0] = num_points[icell_cut][3] = num_points[icell_cut][4] = 6;
	
				y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = dy;
				y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
				z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
				z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;

				x_cut[icell_cut][2][0] = x_cut[icell_cut][2][3] = 0.0;
				x_cut[icell_cut][2][1] = x_cut[icell_cut][2][2] = dx;
				z_cut[icell_cut][2][0] = z_cut[icell_cut][2][1] = 0.0;
				z_cut[icell_cut][2][2] = z_cut[icell_cut][2][3] = dz;
				
				x_cut[icell_cut][5][0] = x_cut[icell_cut][5][3] = 0.0;
				x_cut[icell_cut][5][1] = x_cut[icell_cut][5][2] = dx;
				y_cut[icell_cut][5][0] = y_cut[icell_cut][5][1] = 0.0;
				y_cut[icell_cut][5][2] = y_cut[icell_cut][5][3] = dy;
	
				y_cut[icell_cut][0][4] = y_cut[icell_cut][0][5] = dy;
				y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = 0.0;
				y_cut[icell_cut][0][3] = y_cut[icell_cut][0][2] = y_start-j_cut_start*dy;
				z_cut[icell_cut][0][4] = z_cut[icell_cut][0][3] = H-(k_end-1)*dz;
				z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = 0.0;
				z_cut[icell_cut][0][0] = z_cut[icell_cut][0][5] = dz;
	
				x_cut[icell_cut][3][0] = x_cut[icell_cut][3][5] = 0.0;
				x_cut[icell_cut][3][3] = x_cut[icell_cut][3][4] = dx;
				x_cut[icell_cut][3][1] = x_cut[icell_cut][3][2] = x_start+L-i_end*dx;
				z_cut[icell_cut][3][0] = z_cut[icell_cut][3][1] = H-(k_end-1)*dz;
				z_cut[icell_cut][3][2] = z_cut[icell_cut][3][3] = 0.0;
				z_cut[icell_cut][3][4] = z_cut[icell_cut][3][5] = dz;

				x_cut[icell_cut][4][0] = x_cut[icell_cut][4][5] = 0.0;
				x_cut[icell_cut][4][1] = x_cut[icell_cut][4][2] = dx;
				x_cut[icell_cut][4][3] = x_cut[icell_cut][4][4] = x_start+L-i_end*dx;
				y_cut[icell_cut][4][0] = y_cut[icell_cut][4][1] = 0.0;
				y_cut[icell_cut][4][2] = y_cut[icell_cut][4][3] = dy;
				y_cut[icell_cut][4][4] = y_cut[icell_cut][4][5] = y_start-j_cut_start*dy;
			}

			icell_cut = i_cut_start + (j_cut_end-1)*(nx-1) + k_end*(nx-1)*(ny-1);
			if (icellflag[icell_cut]==7){
				num_points[icell_cut][0] = num_points[icell_cut][3] = num_points[icell_cut][5] = 4;
				num_points[icell_cut][1] = num_points[icell_cut][2] = num_points[icell_cut][4] = 6;
	
				y_cut[icell_cut][0][2] = y_cut[icell_cut][0][3] = dy;
				y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = 0.0;
				z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = 0.0;
				z_cut[icell_cut][0][0] = z_cut[icell_cut][0][3] = dz;
	
				x_cut[icell_cut][3][0] = x_cut[icell_cut][3][3] = 0.0;
				x_cut[icell_cut][3][1] = x_cut[icell_cut][3][2] = dx;
				z_cut[icell_cut][3][0] = z_cut[icell_cut][3][1] = 0.0;
				z_cut[icell_cut][3][2] = z_cut[icell_cut][3][3] = dz;
			
				x_cut[icell_cut][5][0] = x_cut[icell_cut][5][3] = 0.0;
				x_cut[icell_cut][5][1] = x_cut[icell_cut][5][2] = dx;
				y_cut[icell_cut][5][0] = y_cut[icell_cut][5][1] = 0.0;
				y_cut[icell_cut][5][2] = y_cut[icell_cut][5][3] = dy;
		
				y_cut[icell_cut][1][4] = y_cut[icell_cut][1][5] = dy;
				y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
				y_cut[icell_cut][1][3] = y_cut[icell_cut][1][2] = y_start+W-j_end*dy;
				z_cut[icell_cut][1][4] = z_cut[icell_cut][1][3] = 0.0;
				z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = H-(k_end-1)*dz;
				z_cut[icell_cut][1][0] = z_cut[icell_cut][1][5] = dz;
	
				x_cut[icell_cut][2][0] = x_cut[icell_cut][2][5] = 0.0;
				x_cut[icell_cut][2][3] = x_cut[icell_cut][2][4] = dx;
				x_cut[icell_cut][2][1] = x_cut[icell_cut][2][2] = x_start-i_cut_start*dx;
				z_cut[icell_cut][2][0] = z_cut[icell_cut][2][1] = 0.0;
				z_cut[icell_cut][2][2] = z_cut[icell_cut][2][3] = H-(k_end-1)*dz;
				z_cut[icell_cut][2][4] = z_cut[icell_cut][2][5] = dz;

				x_cut[icell_cut][4][0] = x_cut[icell_cut][4][5] = 0.0;
				x_cut[icell_cut][4][1] = x_cut[icell_cut][4][2] = x_start-i_cut_start*dx;
				x_cut[icell_cut][4][3] = x_cut[icell_cut][4][4] = dx;
				y_cut[icell_cut][4][0] = y_cut[icell_cut][4][1] = 0.0;
				y_cut[icell_cut][4][2] = y_cut[icell_cut][4][3] = y_start+W-j_end*dy;
				y_cut[icell_cut][4][4] = y_cut[icell_cut][4][5] = dy;
			}

			icell_cut = (i_cut_end-1) + (j_cut_end-1)*(nx-1) + k_end*(nx-1)*(ny-1);
			if (icellflag[icell_cut]==7){
				num_points[icell_cut][1] = num_points[icell_cut][3] = num_points[icell_cut][5] = 4;
				num_points[icell_cut][0] = num_points[icell_cut][2] = num_points[icell_cut][4] = 6;

				y_cut[icell_cut][1][2] = y_cut[icell_cut][1][3] = dy;
				y_cut[icell_cut][1][1] = y_cut[icell_cut][1][0] = 0.0;
				z_cut[icell_cut][1][2] = z_cut[icell_cut][1][1] = 0.0;
				z_cut[icell_cut][1][0] = z_cut[icell_cut][1][3] = dz;

				x_cut[icell_cut][3][0] = x_cut[icell_cut][3][3] = 0.0;
				x_cut[icell_cut][3][1] = x_cut[icell_cut][3][2] = dx;
				z_cut[icell_cut][3][0] = z_cut[icell_cut][3][1] = 0.0;
				z_cut[icell_cut][3][2] = z_cut[icell_cut][3][3] = dz;
			
				x_cut[icell_cut][5][0] = x_cut[icell_cut][5][3] = 0.0;
				x_cut[icell_cut][5][1] = x_cut[icell_cut][5][2] = dx;
				y_cut[icell_cut][5][0] = y_cut[icell_cut][5][1] = 0.0;
				y_cut[icell_cut][5][2] = y_cut[icell_cut][5][3] = dy;
		
				y_cut[icell_cut][0][4] = y_cut[icell_cut][0][5] = dy;
				y_cut[icell_cut][0][1] = y_cut[icell_cut][0][0] = 0.0;
				y_cut[icell_cut][0][3] = y_cut[icell_cut][0][2] = y_start+W-j_end*dy;
				z_cut[icell_cut][0][4] = z_cut[icell_cut][0][3] = 0.0;
				z_cut[icell_cut][0][2] = z_cut[icell_cut][0][1] = H-(k_end-1)*dz;
				z_cut[icell_cut][0][0] = z_cut[icell_cut][0][5] = dz;
	
				x_cut[icell_cut][2][0] = x_cut[icell_cut][2][5] = 0.0;
				x_cut[icell_cut][2][3] = x_cut[icell_cut][2][4] = dx;
				x_cut[icell_cut][2][1] = x_cut[icell_cut][2][2] = x_start+L-i_end*dx;
				z_cut[icell_cut][2][0] = z_cut[icell_cut][2][1] = H-(k_end-1)*dz;
				z_cut[icell_cut][2][2] = z_cut[icell_cut][2][3] = 0.0;
				z_cut[icell_cut][2][4] = z_cut[icell_cut][2][5] = dz;

				x_cut[icell_cut][4][0] = x_cut[icell_cut][4][5] = 0.0;
				x_cut[icell_cut][4][1] = x_cut[icell_cut][4][2] = x_start+L-i_end*dx;
				x_cut[icell_cut][4][3] = x_cut[icell_cut][4][4] = dx;
				y_cut[icell_cut][4][0] = y_cut[icell_cut][4][1] = y_start+W-j_end*dy;
				y_cut[icell_cut][4][2] = y_cut[icell_cut][4][3] = 0.0;
				y_cut[icell_cut][4][4] = y_cut[icell_cut][4][5] = dy;
			}

	}

/*
Note: The select case portion is almost identical across geometry types 1, 2, and 6.
Difference is that 6 has one more case, type 5. similar to default except sets to 1
instead of 0. Also same as 
*/

	void setCells(float dx, float dy, float dz, int nx, int ny, int nz, int *icellflag) 
	{


		/// defining building and cut-cell indices
		if (fmod(x_start,dx)==0){
			i_start = x_start/dx;
			i_cut_start = i_start;
		} else{
			i_start = 1+x_start/dx;
			i_cut_start = x_start/dx;
		}
		i_end = (x_start+L)/dx;
		i_cut_end = i_end;
		if (fmod((x_start+L),dx)!=0){
			i_cut_end = 1+(x_start+L)/dx;
		}
		if (fmod(y_start,dy)==0){	
			j_start = y_start/dy;
			j_cut_start = j_start;
		} else{
			j_start = 1+y_start/dy;
			j_cut_start = y_start/dy;
		}		
		j_end = (y_start+W)/dy;
		j_cut_end = j_end;
		if (fmod((y_start+W),dy)!=0){
			j_cut_end = 1+(y_start+W)/dy;
		}
		k_end = (H/dz)+1;
		k_cut_end = k_end;
		if (fmod(H,dz)!=0){
			k_cut_end = 2+(H/dz);
		}

		k_start = baseHeight / dz;

    	std::cout << "i_start:" << i_start << "\n";   
   	 	std::cout << "i_end:" << i_end << "\n";       
    	std::cout << "j_start:" << j_start << "\n";  
   	 	std::cout << "j_end:" << j_end << "\n";         
   	 	std::cout << "k_end:" << k_end << "\n";       

   	 	std::cout << "i_cut_start:" << i_cut_start << "\n";  
    	std::cout << "i_cut_end:" << i_cut_end << "\n";      
    	std::cout << "j_cut_start:" << j_cut_start << "\n"; 
    	std::cout << "j_cut_end:" << j_cut_end << "\n";        
    	std::cout << "k_cut_end:" << k_cut_end << "\n"; 

		for (int k = 1; k < k_cut_end; k++){
			for (int j = j_cut_start; j < j_cut_end; j++){
				for (int i = i_cut_start; i < i_cut_end; i++){
	               	icell_cent = i + j*(nx-1) + k*(nx-1)*(ny-1);   /// Lineralized index for cell centered values
					icellflag[icell_cent] = 7;                         /// Set cell index flag to cut-cell
				}
			}
   		}
			

   		for (int k = k_start; k < k_end; k++){
       		for (int j = j_start; j < j_end; j++){
           		for (int i = i_start; i < i_end; i++){
                	icell_cent = i + j*(nx-1) + k*(nx-1)*(ny-1);   /// Lineralized index for cell centered values
					icellflag[icell_cent] = 0;                         /// Set cell index flag to building
				}
			}
    	}

		/// defining ground solid cells
		for (int j = 0; j < ny-1; j++){
			for (int i = 0; i < nx-1; i++){
				int icell_cent = i + j*(nx-1);
				icellflag[icell_cent] = 0.0;
			}
		}
	}

};

