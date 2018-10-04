#pragma once

#include "ParseInterface.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using std::cerr;
using std::endl;
using std::vector;

class Sensor : public ParseInterface
{
private:

public:
		int num_sites;
		std::vector<int> site_blayer_flag;
		std::vector<float> site_one_overL;
		std::vector<float> site_xcoord;
		std::vector<float> site_ycoord;
		std::vector<float> site_wind_dir;

		std::vector<float> site_z0;
		std::vector<float> site_z_ref;
		std::vector<float> site_U_ref;
		std::vector<float> site_theta;
		
		
		//virtual void inputWindProfile(float, float, float, int, int, int, double*, double*, double*, float*, float*, float*);

	virtual void parseValues()
	{
		parsePrimitive<int>(true, num_sites, "numberofSites");
		//parsePrimitive<int>(true, siteCoords, "siteCoords");
		parseMultiPrimitives<float>(true, site_xcoord, "site_xcoord");
		parseMultiPrimitives<float>(true, site_ycoord, "site_ycoord");
		parseMultiPrimitives<int>(true, site_blayer_flag, "boundaryLayerFlag");
		parseMultiPrimitives<float>(true, site_z0, "siteZ0");
		parseMultiPrimitives<float>(true, site_one_overL, "reciprocal");
		parseMultiPrimitives<float>(true, site_z_ref, "height");
		parseMultiPrimitives<float>(true, site_U_ref, "speed");
		parseMultiPrimitives<float>(true, site_wind_dir, "direction");

	}

	void inputWindProfile(float dx, float dy, float dz, int nx, int ny, int nz, double *u0, double *v0, double *w0)
	{

		float *x1, *y1, *z1;
		x1 = new float [nx];
		y1 = new float [ny];
		z1 = new float [nz];
		for ( int i = 0; i < nx; i++){
			x1[i] = (i+0.5)*dx;         /// Location of face centers in x-dir
		}
		for ( int j = 0; j < ny; j++){
			y1[j] = (j+0.5)*dy;         /// Location of face centers in y-dir
		}	
		for ( int k = 0; k < nz; k++){
			z1[k] = (k-0.5)*dz;         /// Location of face centers in z-dir
		}	
		double **u_prof, **v_prof;
		u_prof = new double* [num_sites];
		v_prof = new double* [num_sites];

		for (int i = 0; i < num_sites; i++){
			u_prof[i] = new double [nz];
			v_prof[i] = new double [nz];
		}

		float domain_rotation = 0.0;
		float theta = (domain_rotation*M_PI/180);
		float psi, x_temp, u_star; 
		float rc_sum, rc_value, xc, yc, rc, dn, lamda, s_gamma;
		float sum_wm, sum_wu, sum_wv;
		int iwork, jwork,rc_val;
		float dxx, dyy, u12, u34, v12, v34;
		float *u0_int, *v0_int, *site_theta;
		int icell_face, icell_cent;
		u0_int = new float [num_sites]();
		v0_int = new float [num_sites]();	
		site_theta = new float [num_sites]();
		float vk = 0.4;			/// Von Karman's constant

		for (int i = 0 ; i < num_sites; i++){
			site_theta[i] = (270.0-site_wind_dir[i])*M_PI/180.0;
			if (site_blayer_flag[i] == 0){
				for (int k = 1; k < nz; k++){
					u_prof[i][k] = 0.0;
					v_prof[i][k] = 0.0;
				}
			}

			if (site_blayer_flag[i] == 1){
				for (int k = 1; k < nz; k++){
					if (k == 1){
						if (site_z_ref[i]*site_one_overL[i] >= 0){
							psi = 4.7*site_z_ref[i]*site_one_overL[i];
						}
						else {
							x_temp = pow((1-15*site_z_ref[i]*site_one_overL[i]),0.25);
							psi = -2*log(0.5*(1+x_temp))-log(0.5*(1+pow(x_temp,2.0)))+2*atan(x_temp)-0.5*M_PI;
						}
					
						u_star = site_U_ref[i]*vk/(log((site_z_ref[i]+site_z0[i])/site_z0[i])+psi);
					}
					if (z1[k]*site_one_overL[i] >= 0){
						psi = 4.7*z1[k]*site_one_overL[i];
					}
					else {
						x_temp = pow((1-15*z1[k]*site_one_overL[i]),0.25);
						psi = -2*log(0.5*(1+x_temp))-log(0.5*(1+pow(x_temp,2.0)))+2*atan(x_temp)-0.5*M_PI;
					}
	
					u_prof[i][k] = (cos(site_theta[i])*u_star/vk)*(log((z1[k]+site_z0[i])/site_z0[i])+psi);
					v_prof[i][k] = (sin(site_theta[i])*u_star/vk)*(log((z1[k]+site_z0[i])/site_z0[i])+psi);				
				}
			}

			if (site_blayer_flag[i] == 2){
				for (int k = 1; k < nz; k++){
					u_prof[i][k] = cos(site_theta[i])*site_U_ref[i]*pow((z1[k]/site_z_ref[i]),site_z0[i]);
					v_prof[i][k] = sin(site_theta[i])*site_U_ref[i]*pow((z1[k]/site_z_ref[i]),site_z0[i]);
				}
			}
		}

		double ***wm, ***wms;
		wm = new double** [num_sites]();
		wms = new double** [num_sites]();
		for (int i = 0; i < num_sites; i++){
			wm[i] = new double* [nx]();
			wms[i] = new double* [nx]();
			for (int j = 0; j < nx; j++){
				wm[i][j] = new double [ny]();
				wms[i][j] = new double [ny]();
			}
		}

		if (num_sites == 1){
			for ( int k = 0; k < nz; k++){
				for (int j = 0; j < ny; j++){
					for (int i = 0; i < nx; i++){
					
						icell_face = i + j*nx + k*nx*ny;   /// Lineralized index for cell faced values                                
		                u0[icell_face] = u_prof[0][k];
						v0[icell_face] = v_prof[0][k];
						w0[icell_face] = 0.0;         /// Perpendicular wind direction
					}
    	        }
    	    }
    	}
		else {
			rc_sum = 0.0;
			for (int i = 0; i < num_sites; i++){
				rc_val = 1000000.0;
				for (int ii = 0; ii < num_sites; ii++){
					xc = site_xcoord[ii] - site_xcoord[i];
					yc = site_ycoord[ii] - site_ycoord[i];
					rc = sqrt(pow(xc,2.0)+pow(yc,2.0));
					if (rc < rc_val && ii != i){
						rc_val = rc;
					}
				}
				rc_sum += rc_val;
			}
			dn = rc_sum/num_sites;
			lamda = 5.052*pow((2*dn/M_PI),2.0);
			s_gamma = 0.2;
			for (int j=0; j<ny; j++){
				for (int i=0; i<nx; i++){
					sum_wm = 0.0;
					for (int ii=0; ii<num_sites; ii++){
						wm[ii][i][j] = exp(-1/lamda*pow(site_xcoord[ii]-x1[i]-dx,2.0)-1/lamda*pow(site_ycoord[ii]-y1[j]-dy,2.0));
						wms[ii][i][j] = exp(-1/(s_gamma*lamda)*pow(site_xcoord[ii]-x1[i]-dx,2.0)-1/(s_gamma*lamda)*pow(site_ycoord[ii]-y1[j]-dy,2.0));
						sum_wm += wm[ii][i][j];
					}
					if (sum_wm == 0){
						for (int ii = 0; ii<num_sites; ii++){
							wm[ii][i][j] = 1e-20;
						}
					}
				}
			}
	
			for (int k=1; k<nz; k++){
				for (int j=0; j<ny; j++){
					for (int i=0; i<nx; i++){
						sum_wu = 0.0;
						sum_wv = 0.0;
						sum_wm = 0.0;
						for (int ii=0; ii<num_sites; ii++){
							sum_wu += wm[ii][i][j]*u_prof[ii][k];
							sum_wv += wm[ii][i][j]*v_prof[ii][k];	
							sum_wm += wm[ii][i][j];
						}
						icell_face = i + j*nx + k*nx*ny;   /// Lineralized index for cell faced values 
						u0[icell_face] = sum_wu/sum_wm;
						v0[icell_face] = sum_wv/sum_wm;	
						w0[icell_face] = 0.0;
					}
				}
	
				for (int ii=0; ii<num_sites; ii++){
					if(site_xcoord[ii]>0 && site_xcoord[ii]<(nx-1)*dx && site_ycoord[ii]>0 && site_ycoord[ii]>(ny-1)*dy){
						for (int j=0; j<ny; j++){
							if (y1[j]<site_ycoord[ii]){
								jwork = j;
							}
						}
						for (int i=0; i<nx; i++){
							if (x1[i]<site_xcoord[ii]){
								iwork = i;
							}
						} 
						dxx = site_xcoord[ii]-x1[iwork];
						dyy = site_ycoord[ii]-y1[jwork];
						int index_work = iwork+jwork*nx+k*nx*ny;
						u12 = (1-dxx/dx)*u0[index_work+nx]+(dxx/dx)*u0[index_work+1+nx];
						u34 = (1-dxx/dx)*u0[index_work]+(dxx/dx)*u0[index_work+1];
						u0_int[ii] = (dyy/dy)*u12+(1-dyy/dy)*u34;
		
						v12 = (1-dxx/dx)*v0[index_work+nx]+(dxx/dx)*v0[index_work+1+nx];
						v34 = (1-dxx/dx)*v0[index_work]+(dxx/dx)*v0[index_work+1];
						v0_int[ii] = (dyy/dy)*v12+(1-dyy/dy)*v34;
					}
					else{
						u0_int[ii] = u_prof[ii][k];
						v0_int[ii] = v_prof[ii][k];
					}
				}
	
				for (int j=0; j<ny; j++){
					for (int i=0; i<nx; i++){
						sum_wu = 0.0;
						sum_wv = 0.0;
						sum_wm = 0.0;
						for (int ii=0; ii<num_sites; ii++){
							sum_wu += wm[ii][i][j]*u_prof[ii][k];
							sum_wv += wm[ii][i][j]*v_prof[ii][k];	
							sum_wm += wm[ii][i][j];
						}
						if (sum_wm != 0){
							icell_face = i + j*nx + k*nx*ny;   /// Lineralized index for cell faced values 
							u0[icell_face] = u0[icell_face]+sum_wu/sum_wm;
							v0[icell_face] = v0[icell_face]+sum_wv/sum_wm;
						}	
					}
				}
			}
		}
	}

};


