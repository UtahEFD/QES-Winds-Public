/*
 * QES-Winds
 *
 * Copyright (c) 2021 University of Utah
 * Copyright (c) 2021 University of Minnesota Duluth
 *
 * Copyright (c) 2021 Behnam Bozorgmehr
 * Copyright (c) 2021 Jeremy A. Gibbs
 * Copyright (c) 2021 Fabien Margairaz
 * Copyright (c) 2021 Eric R. Pardyjak
 * Copyright (c) 2021 Zachary Patterson
 * Copyright (c) 2021 Rob Stoll
 * Copyright (c) 2021 Pete Willemsen
 *
 * This file is part of QES-Winds
 *
 * GPL-3.0 License
 *
 * QES-Winds is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * QES-Winds is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QES-Winds. If not, see <https://www.gnu.org/licenses/>.
 *
 */


#include <math.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <limits>

#include "Sensor.h"

#include "WINDSInputData.h"
#include "WINDSGeneralData.h"


using namespace std;




void Sensor::inputWindProfile(const WINDSInputData *WID, WINDSGeneralData *WGD, int index, int solverType)
{

	const float vk = 0.4;			/// Von Karman's constant
	float canopy_d, u_H;
	float site_UTM_x, site_UTM_y;
	float site_lon, site_lat;
	float wind_dir, z0_new, z0_high, z0_low;
	float psi, psi_first, x_temp, u_star;
	float u_new, u_new_low, u_new_high;
	int log_flag, iter, id;
	float a1, a2, a3;
	float site_mag;
	float blending_height = 0.0, average__one_overL = 0.0;
	int max_terrain = 1;
	std::vector<float> x,y;

	int num_sites = WID->metParams->sensors.size();
	std::vector<std::vector<float>> u_prof(num_sites, std::vector<float>(WGD->nz,0.0));
	std::vector<std::vector<float>> v_prof(num_sites, std::vector<float>(WGD->nz,0.0));
	int icell_face, icell_cent;

	std::vector<int> site_i(num_sites,0);
	std::vector<int> site_j(num_sites,0);
	std::vector<int> site_id(num_sites,0);
	std::vector<float> site_theta(num_sites,0.0);

	// Loop through all sites and create velocity profiles (WGD->u0,WGD->v0)
	for (auto i = 0 ; i < num_sites; i++)
	{
		float convergence = 0.0;
	  site_i[i] = WID->metParams->sensors[i]->site_xcoord/WGD->dx;
		site_j[i] = WID->metParams->sensors[i]->site_ycoord/WGD->dy;
		site_id[i] = site_i[i] + site_j[i]*(WGD->nx-1);
		for (auto j=0; j<WID->metParams->sensors[i]->TS[index]->site_z_ref.size(); j++)
		{
			WID->metParams->sensors[i]->TS[index]->site_z_ref[j] += WGD->terrain[site_id[i]];
		}
		int id = 1;
		int counter = 0;
		if (WID->metParams->sensors[i]->TS[index]->site_z_ref[0] > 0)
		{
			blending_height += WID->metParams->sensors[i]->TS[index]->site_z_ref[0]/num_sites;
		}
		else
		{
			if (WID->metParams->sensors[i]->TS[index]->site_blayer_flag == 4 )
			{
				while (id<WID->metParams->sensors[i]->TS[index]->site_z_ref.size() && WID->metParams->sensors[i]->TS[index]->site_z_ref[id]>0 && counter<1)
				{
					blending_height += WID->metParams->sensors[i]->TS[index]->site_z_ref[id]/num_sites;
					counter += 1;
					id += 1;
				}
			}
		}

		average__one_overL += WID->metParams->sensors[i]->TS[index]->site_one_overL/num_sites;
		if (WID->simParams->UTMx != 0 && WID->simParams->UTMy != 0)
		{
			if (WID->metParams->sensors[i]->site_coord_flag == 1)
			{
				WID->metParams->sensors[i]->site_UTM_x = WID->metParams->sensors[i]->site_xcoord * acos(WGD->theta) + WID->metParams->sensors[i]->site_ycoord * asin(WGD->theta) + WID->simParams->UTMx;
				WID->metParams->sensors[i]->site_UTM_y = WID->metParams->sensors[i]->site_xcoord * asin(WGD->theta) + WID->metParams->sensors[i]->site_ycoord * acos(WGD->theta) + WID->simParams->UTMy;
				WID->metParams->sensors[i]->site_UTM_zone = WID->simParams->UTMZone;
				// Calling UTMConverter function to convert UTM coordinate to lat/lon and vice versa (located in Sensor.cpp)
				UTMConverter (WID->metParams->sensors[i]->site_lon, WID->metParams->sensors[i]->site_lat, WID->metParams->sensors[i]->site_UTM_x, WID->metParams->sensors[i]->site_UTM_y, WID->metParams->sensors[i]->site_UTM_zone, 1);
			}

			if (WID->metParams->sensors[i]->site_coord_flag == 2)
			{
				// Calling UTMConverter function to convert UTM coordinate to lat/lon and vice versa (located in Sensor.cpp)
				UTMConverter (WID->metParams->sensors[i]->site_lon, WID->metParams->sensors[i]->site_lat, WID->metParams->sensors[i]->site_UTM_x, WID->metParams->sensors[i]->site_UTM_y, WID->metParams->sensors[i]->site_UTM_zone, 1);
			}

			getConvergence(WID->metParams->sensors[i]->site_lon, WID->metParams->sensors[i]->site_lat, WID->metParams->sensors[i]->site_UTM_zone, convergence);
		}

		site_theta[i] = (270.0-WID->metParams->sensors[i]->TS[index]->site_wind_dir[0])*M_PI/180.0;

		// If site has a uniform velocity profile
		if (WID->metParams->sensors[i]->TS[index]->site_blayer_flag == 0)
		{
			for (auto k = WGD->terrain_id[site_id[i]]; k < WGD->nz; k++)
			{
				u_prof[i][k] = cos(site_theta[i])*WID->metParams->sensors[i]->TS[index]->site_U_ref[0];
				v_prof[i][k] = sin(site_theta[i])*WID->metParams->sensors[i]->TS[index]->site_U_ref[0];
			}
		}
		// Logarithmic velocity profile
		if (WID->metParams->sensors[i]->TS[index]->site_blayer_flag == 1)
		{
                    // This loop should be bounded by size of the z
                    // vector, and not WGD->nz since z.size can be equal to
                    // WGD->nz+1 from what I can tell.  We access z[k]
                    // below...
                    for (auto k = WGD->terrain_id[site_id[i]]; k < WGD->z.size(); k++)
			{
				if (k == WGD->terrain_id[site_id[i]])
				{
					if (WID->metParams->sensors[i]->TS[index]->site_z_ref[0]*WID->metParams->sensors[i]->TS[index]->site_one_overL >= 0)
					{
						psi = 4.7*WID->metParams->sensors[i]->TS[index]->site_z_ref[0]*WID->metParams->sensors[i]->TS[index]->site_one_overL;
					}
					else
					{
						x_temp = pow((1.0-15.0*WID->metParams->sensors[i]->TS[index]->site_z_ref[0]*WID->metParams->sensors[i]->TS[index]->site_one_overL),0.25);
						psi = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
					}

					u_star = WID->metParams->sensors[i]->TS[index]->site_U_ref[0]*vk/(log((WID->metParams->sensors[i]->TS[index]->site_z_ref[0]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0)+psi);
				}
				if (WGD->z[k]*WID->metParams->sensors[i]->TS[index]->site_one_overL >= 0)
				{
					psi = 4.7*WGD->z[k]*WID->metParams->sensors[i]->TS[index]->site_one_overL;
				}
				else
				{
					x_temp = pow((1.0-15.0*WGD->z[k]*WID->metParams->sensors[i]->TS[index]->site_one_overL),0.25);
					psi = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
				}

				u_prof[i][k] = (cos(site_theta[i])*u_star/vk)*(log((WGD->z[k]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0)+psi);
				v_prof[i][k] = (sin(site_theta[i])*u_star/vk)*(log((WGD->z[k]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0)+psi);
			}
		}

		// Exponential velocity profile
		if (WID->metParams->sensors[i]->TS[index]->site_blayer_flag == 2)
		{
			for (auto k = WGD->terrain_id[site_id[i]]; k < WGD->nz; k++)
			{
				u_prof[i][k] = cos(site_theta[i])*WID->metParams->sensors[i]->TS[index]->site_U_ref[0]*pow((WGD->z[k]/WID->metParams->sensors[i]->TS[index]->site_z_ref[0]),WID->metParams->sensors[i]->TS[index]->site_z0);
				v_prof[i][k] = sin(site_theta[i])*WID->metParams->sensors[i]->TS[index]->site_U_ref[0]*pow((WGD->z[k]/WID->metParams->sensors[i]->TS[index]->site_z_ref[0]),WID->metParams->sensors[i]->TS[index]->site_z0);
			}
		}

		// Canopy velocity profile
		if (WID->metParams->sensors[i]->TS[index]->site_blayer_flag == 3)
		{
			for (auto k = WGD->terrain_id[site_id[i]]; k< WGD->nz; k++)
			{
				if (k == WGD->terrain_id[site_id[i]])
				{
					if (WID->metParams->sensors[i]->TS[index]->site_z_ref[0]*WID->metParams->sensors[i]->TS[index]->site_one_overL > 0)
					{
						psi = 4.7*WID->metParams->sensors[i]->TS[index]->site_z_ref[0]*WID->metParams->sensors[i]->TS[index]->site_one_overL;
					}
					else
					{
						x_temp = pow((1.0-15.0*WID->metParams->sensors[i]->TS[index]->site_z_ref[0]*WID->metParams->sensors[i]->TS[index]->site_one_overL),0.25);
						psi = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
					}
					u_star = WID->metParams->sensors[i]->TS[index]->site_U_ref[0]*vk/(log(WID->metParams->sensors[i]->TS[index]->site_z_ref[0]/WID->metParams->sensors[i]->TS[index]->site_z0)+psi);
					canopy_d = WGD->canopyBisection(u_star, WID->metParams->sensors[i]->TS[index]->site_z0, WID->metParams->sensors[i]->TS[index]->site_canopy_H, WID->metParams->sensors[i]->TS[index]->site_atten_coeff, vk, psi);
					if (WID->metParams->sensors[i]->TS[index]->site_canopy_H*WID->metParams->sensors[i]->TS[index]->site_one_overL > 0)
					{
						psi = 4.7*(WID->metParams->sensors[i]->TS[index]->site_canopy_H-canopy_d)*WID->metParams->sensors[i]->TS[index]->site_one_overL;
					}
					else
					{
						x_temp = pow((1.0-15.0*(WID->metParams->sensors[i]->TS[index]->site_canopy_H-canopy_d)*WID->metParams->sensors[i]->TS[index]->site_one_overL),0.25);
						psi = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
					}
					u_H = (u_star/vk)*(log((WID->metParams->sensors[i]->TS[index]->site_canopy_H-canopy_d)/WID->metParams->sensors[i]->TS[index]->site_z0)+psi);
					if (WID->metParams->sensors[i]->TS[index]->site_z_ref[0] < WID->metParams->sensors[i]->TS[index]->site_canopy_H)
					{
						WID->metParams->sensors[i]->TS[index]->site_U_ref[0] /= u_H*exp(WID->metParams->sensors[i]->TS[index]->site_atten_coeff*(WID->metParams->sensors[i]->TS[index]->site_z_ref[0]/WID->metParams->sensors[i]->TS[index]->site_canopy_H)-1.0);
					}
					else
					{
						if (WID->metParams->sensors[i]->TS[index]->site_z_ref[0]*WID->metParams->sensors[i]->TS[index]->site_one_overL > 0)
						{
							psi = 4.7*(WID->metParams->sensors[i]->TS[index]->site_z_ref[0]-canopy_d)*WID->metParams->sensors[i]->TS[index]->site_one_overL;
						}
						else
						{
							x_temp = pow(1.0-15.0*(WID->metParams->sensors[i]->TS[index]->site_z_ref[0]-canopy_d)*WID->metParams->sensors[i]->TS[index]->site_one_overL,0.25);
							psi = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
						}
						WID->metParams->sensors[i]->TS[index]->site_U_ref[0] /= ((u_star/vk)*(log((WID->metParams->sensors[i]->TS[index]->site_z_ref[0]-canopy_d)/WID->metParams->sensors[i]->TS[index]->site_z0)+psi));
					}
					u_star *= WID->metParams->sensors[i]->TS[index]->site_U_ref[0];
					u_H *= WID->metParams->sensors[i]->TS[index]->site_U_ref[0];
				}

				if (WGD->z[k] < WID->metParams->sensors[i]->TS[index]->site_canopy_H)
				{
					u_prof[i][k] = cos(site_theta[i]) * u_H*exp(WID->metParams->sensors[i]->TS[index]->site_atten_coeff*((WGD->z[k]/WID->metParams->sensors[i]->TS[index]->site_canopy_H) -1.0));
					v_prof[i][k] = sin(site_theta[i]) * u_H*exp(WID->metParams->sensors[i]->TS[index]->site_atten_coeff*((WGD->z[k]/WID->metParams->sensors[i]->TS[index]->site_canopy_H) -1.0));
				}
				if (WGD->z[k] > WID->metParams->sensors[i]->TS[index]->site_canopy_H)
				{
					if (WGD->z[k]*WID->metParams->sensors[i]->TS[index]->site_one_overL > 0)
					{
						psi = 4.7*(WGD->z[k]-canopy_d)*WID->metParams->sensors[i]->TS[index]->site_one_overL;
					}
					else
					{
						x_temp = pow(1.0-15.0*(WGD->z[k]-canopy_d)*WID->metParams->sensors[i]->TS[index]->site_one_overL,0.25);
						psi = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
					}
					u_prof[i][k] = (cos(site_theta[i])*u_star/vk)*(log((WGD->z[k]-canopy_d)/WID->metParams->sensors[i]->TS[index]->site_z0)+psi);
					v_prof[i][k] = (sin(site_theta[i])*u_star/vk)*(log((WGD->z[k]-canopy_d)/WID->metParams->sensors[i]->TS[index]->site_z0)+psi);
				}
			}
		}

		// Data entry profile (WRF output)
		if (WID->metParams->sensors[i]->TS[index]->site_blayer_flag == 4)
		{
			int z_size = WID->metParams->sensors[i]->TS[index]->site_z_ref.size();
			int ii = -1;
			site_theta[i] = (270.0-WID->metParams->sensors[i]->TS[index]->site_wind_dir[0])*M_PI/180.0;

                        // Needs to be nz-1 for [0, n-1] indexing
			for (auto k=WGD->terrain_id[site_id[i]]; k<WGD->nz-1; k++)
			{
				if (WGD->z[k] < WID->metParams->sensors[i]->TS[index]->site_z_ref[0] || z_size == 1)
				{
					u_prof[i][k] = (WID->metParams->sensors[i]->TS[index]->site_U_ref[0]*cos(site_theta[i])/log((WID->metParams->sensors[i]->TS[index]->site_z_ref[0]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0))
													*log((WGD->z[k]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0);
					v_prof[i][k] = (WID->metParams->sensors[i]->TS[index]->site_U_ref[0]*sin(site_theta[i])/log((WID->metParams->sensors[i]->TS[index]->site_z_ref[0]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0))
													*log((WGD->z[k]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0);
				}
				else
				{

					if ( (ii < z_size-2) && (WGD->z[k] >= WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]))
					{
						ii += 1;
						if (abs(WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii+1]-WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii]) > 180.0)
						{
							if (WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii+1] > WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii])
							{
								wind_dir = (WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii+1]-360.0-WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii+1])
														/(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]);
							}
							else
							{
								wind_dir = (WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii+1]+360.0-WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii+1])
														/(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]);
							}
						}
						else
						{
							wind_dir = (WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii+1]-WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii])
													/(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]);
						}
						z0_high = 20.0;
						u_star = vk*WID->metParams->sensors[i]->TS[index]->site_U_ref[ii]/log((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]+z0_high)/z0_high);
						u_new_high = (u_star/vk)*log((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]+z0_high)/z0_high);
						z0_low = 1e-9;
						u_star = vk*WID->metParams->sensors[i]->TS[index]->site_U_ref[ii]/log((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]+z0_low)/z0_low);
						u_new_low = (u_star/vk)*log((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]+z0_low)/z0_low);

						if (WID->metParams->sensors[i]->TS[index]->site_U_ref[ii+1] > u_new_low && WID->metParams->sensors[i]->TS[index]->site_U_ref[ii+1] < u_new_high)
						{
							log_flag = 1;
							iter = 0;
							u_star = vk*WID->metParams->sensors[i]->TS[index]->site_U_ref[ii]/log((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0);
							u_new = (u_star/vk)*log((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]+WID->metParams->sensors[i]->TS[index]->site_z0)/WID->metParams->sensors[i]->TS[index]->site_z0);
							while (iter < 200 && abs(u_new-WID->metParams->sensors[i]->TS[index]->site_U_ref[ii]) > 0.0001*WID->metParams->sensors[i]->TS[index]->site_U_ref[ii])
							{
								iter += 1;
								z0_new = 0.5*(z0_low+z0_high);
								u_star = vk*WID->metParams->sensors[i]->TS[index]->site_U_ref[ii]/log((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]+z0_new)/z0_new);
								u_new = (u_star/vk)*log((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]+z0_new)/z0_new);
								if (u_new > WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1])
								{
									z0_high = z0_new;
								}
								else
								{
									z0_low = z0_new;
								}
							}
						}
						else
						{
							log_flag = 0;
							if (ii < z_size-2)
							{
								a1 = ((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii])
									 	*(WID->metParams->sensors[i]->TS[index]->site_U_ref[ii+2]-WID->metParams->sensors[i]->TS[index]->site_U_ref[ii])
									 	+(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+2])
									 	*(WID->metParams->sensors[i]->TS[index]->site_U_ref[ii+1]-WID->metParams->sensors[i]->TS[index]->site_U_ref[ii]))
									 	/((WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii])
									 	*(pow(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+2],2.0)-pow(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii],2.0))
									 	+(pow(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1],2.0)-pow(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii],2.0))
									 	*(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+2]));
							}
							else
							{
								a1 = 0.0;
							}
							a2 = ((WID->metParams->sensors[i]->TS[index]->site_U_ref[ii+1]-WID->metParams->sensors[i]->TS[index]->site_U_ref[ii])
								 	-a1*(pow(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1],2.0)-pow(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii],2.0)))
								 	/(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii+1]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii]);
							a3 = WID->metParams->sensors[i]->TS[index]->site_U_ref[ii]-a1*pow(WID->metParams->sensors[i]->TS[index]->site_z_ref[ii],2.0)
								 	-a2*WID->metParams->sensors[i]->TS[index]->site_z_ref[ii];
						}
					}
					if (log_flag == 1)
					{
						site_mag = (u_star/vk)*log((WGD->z[k]+z0_new)/z0_new);
					}
					else
					{
						site_mag = a1*pow(WGD->z[k], 2.0)+a2*WGD->z[k]+a3;
					}
					site_theta[i] = (270.0-(WID->metParams->sensors[i]->TS[index]->site_wind_dir[ii]+
											wind_dir*(WGD->z[k]-WID->metParams->sensors[i]->TS[index]->site_z_ref[ii])))*M_PI/180.0;
					u_prof[i][k] = site_mag*cos(site_theta[i]);
					v_prof[i][k] = site_mag*sin(site_theta[i]);
				}
			}
		}
	}

	x.resize( WGD->nx );
	for (size_t i=0; i<WGD->nx; i++)
	{
		x[i] = (i-0.5)*WGD->dx;          /**< Location of face centers in x-dir */
	}

	y.resize( WGD->ny );
	for (auto j=0; j<WGD->ny; j++)
	{
		y[j] = (j-0.5)*WGD->dy;          /**< Location of face centers in y-dir */
	}


	if (num_sites == 1)
	{
		for ( auto k = 0; k < WGD->nz; k++)
		{
			for (auto j = 0; j < WGD->ny; j++)
			{
				for (auto i = 0; i < WGD->nx; i++)
				{

					icell_face = i + j*WGD->nx + k*WGD->nx*WGD->ny;   /// Lineralized index for cell faced values
		      WGD->u0[icell_face] = u_prof[0][k];
					WGD->v0[icell_face] = v_prof[0][k];
					//WGD->w0[icell_face] = 0.0;         /// Perpendicular wind direction
				}
			}
   	}
  }

	// If number of sites are more than one
	// Apply 2D Barnes scheme to interpolate site velocity profiles to the whole domain
	else
	{
		if (solverType == 1)
		{
			auto startBarnesCPU = std::chrono::high_resolution_clock::now();
			BarnesInterpolationCPU (WID, WGD, u_prof, v_prof);
			auto finishBarnesCPU = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsedBarnesCPU = finishBarnesCPU - startBarnesCPU;
	    std::cout << "Elapsed time for Barnes interpolation on CPU: " << elapsedBarnesCPU.count() << " s\n";
		}
		else
		{
			auto startBarnesGPU = std::chrono::high_resolution_clock::now();
			BarnesInterpolationGPU (WID, WGD, u_prof, v_prof);
			auto finishBarnesGPU = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsedBarnesGPU = finishBarnesGPU - startBarnesGPU;
	    std::cout << "Elapsed time for Barnes interpolation on GPU: " << elapsedBarnesGPU.count() << " s\n";
		}

	}


  float z0_domain;
	if (WID->metParams->z0_domain_flag == 1)
	{
		float sum_z0=0.0;
		float z0_effective;
		int height_id, blending_id, max_terrain_id=0;
		std::vector<float> blending_velocity, blending_theta;
		for (auto i=0; i<WGD->nx; i++)
		{
			for (auto j=0; j<WGD->ny; j++)
			{
				id = i+j*WGD->nx;
				if (WGD->terrain_id[id] > max_terrain)
				{
					max_terrain = WGD->terrain_id[id];
					max_terrain_id = i+j*(WGD->nx-1);
				}
			}
		}

		for (auto i=0; i<WGD->nx; i++)
		{
			for (auto j=0; j<WGD->ny; j++)
			{
				id = i+j*WGD->nx;
				sum_z0 += log( ((WGD->z0_domain_u[id]+WGD->z0_domain_v[id])/2) + WGD->z[WGD->terrain_id[id]]);
			}
		}
		z0_effective = exp(sum_z0/(WGD->nx*WGD->ny));
		blending_height = blending_height+WGD->terrain[max_terrain_id];
		for (auto k=0; k<WGD->z.size(); k++)
		{
			height_id = k+1;
			if (blending_height < WGD->z[k+1])
			{
				break;
			}
		}

		blending_velocity.resize( WGD->nx*WGD->ny, 0.0 );
		blending_theta.resize( WGD->nx*WGD->ny, 0.0 );

		for (auto i=0; i<WGD->nx; i++)
		{
			for (auto j=0; j<WGD->ny; j++)
			{
				blending_id = i+j*WGD->nx+height_id*WGD->nx*WGD->ny;
				id = i+j*WGD->nx;
				blending_velocity[id] = sqrt(pow(WGD->u0[blending_id],2.0)+pow(WGD->v0[blending_id],2.0));
				blending_theta[id] = atan2(WGD->v0[blending_id],WGD->u0[blending_id]);
			}
		}

		for (auto i=0; i<WGD->nx; i++)
		{
			for (auto j=0; j<WGD->ny; j++)
			{
				id = i+j*WGD->nx;
				if (blending_height*average__one_overL >= 0)
				{
					psi_first = 4.7*blending_height*average__one_overL;
				}
				else
				{
					x_temp = pow((1.0-15.0*blending_height*average__one_overL),0.25);
					psi_first = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
				}

				for (auto k = WGD->terrain_id[id]; k < height_id; k++)
				{
					if (WGD->z[k]*average__one_overL >= 0)
					{
						psi = 4.7*WGD->z[k]*average__one_overL;
					}
					else
					{
						x_temp = pow((1.0-15.0*WGD->z[k]*average__one_overL),0.25);
						psi = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
					}
					icell_face = i+j*WGD->nx+k*WGD->nx*WGD->ny;
          z0_domain = (WGD->z0_domain_u[id] + WGD->z0_domain_v[id])/2;
					u_star = blending_velocity[id]*vk/(log((blending_height+z0_domain)/z0_domain)+psi_first);
					WGD->u0[icell_face] = (cos(blending_theta[id])*u_star/vk)*(log((WGD->z[k]+WGD->z0_domain_u[id])/WGD->z0_domain_u[id])+psi);
					WGD->v0[icell_face] = (sin(blending_theta[id])*u_star/vk)*(log((WGD->z[k]+WGD->z0_domain_v[id])/WGD->z0_domain_v[id])+psi);
				}

				for (auto k = height_id+1; k < WGD->nz-1; k++)
				{
					if (WGD->z[k]*average__one_overL >= 0)
					{
						psi = 4.7*WGD->z[k]*average__one_overL;
					}
					else
					{
						x_temp = pow((1.0-15.0*WGD->z[k]*average__one_overL),0.25);
						psi = -2.0*log(0.5*(1.0+x_temp))-log(0.5*(1.0+pow(x_temp,2.0)))+2.0*atan(x_temp)-0.5*M_PI;
					}
					icell_face = i+j*WGD->nx+k*WGD->nx*WGD->ny;
					u_star = blending_velocity[id]*vk/(log((blending_height+z0_effective)/z0_effective)+psi_first);
					WGD->u0[icell_face] = (cos(blending_theta[id])*u_star/vk)*(log((WGD->z[k]+z0_effective)/z0_effective)+psi);
					WGD->v0[icell_face] = (sin(blending_theta[id])*u_star/vk)*(log((WGD->z[k]+z0_effective)/z0_effective)+psi);
				}

			}
		}

	}

}

void Sensor::BarnesInterpolationCPU(const WINDSInputData *WID, WINDSGeneralData *WGD, std::vector<std::vector<float>> u_prof, std::vector<std::vector<float>> v_prof)
{
	std::vector<float> x,y;
	x.resize( WGD->nx );
	for (size_t i=0; i<WGD->nx; i++)
	{
		x[i] = (i-0.5)*WGD->dx;          /**< Location of face centers in x-dir */
	}

	y.resize( WGD->ny );
	for (auto j=0; j<WGD->ny; j++)
	{
		y[j] = (j-0.5)*WGD->dy;          /**< Location of face centers in y-dir */
	}

	float rc_sum, rc_val, xc, yc, rc;
	float dn, lamda, s_gamma;
	float sum_wm, sum_wu, sum_wv;
	float dxx, dyy, u12, u34, v12, v34;
	int icell_face, icell_cent;
	int num_sites = WID->metParams->sensors.size();
	std::vector<float> u0_int(num_sites,0.0);
	std::vector<float> v0_int(num_sites,0.0);
	std::vector<std::vector<std::vector<float>>> wm(num_sites, std::vector<std::vector<float>>(WGD->nx, std::vector<float>(WGD->ny,0.0)));
	std::vector<std::vector<std::vector<float>>> wms(num_sites, std::vector<std::vector<float>>(WGD->nx, std::vector<float>(WGD->ny,0.0)));
	int iwork = 0, jwork = 0;

	rc_sum = 0.0;
	for (auto i = 0; i < num_sites; i++)
	{
		rc_val = 1000000.0;
		for (auto ii = 0; ii < num_sites; ii++)
		{
			xc = WID->metParams->sensors[ii]->site_xcoord - WID->metParams->sensors[i]->site_xcoord;
			yc = WID->metParams->sensors[ii]->site_ycoord - WID->metParams->sensors[i]->site_ycoord;
			rc = sqrt(pow(xc,2.0)+pow(yc,2.0));
			if (rc < rc_val && ii != i){
				rc_val = rc;
			}
		}
		rc_sum = rc_sum+rc_val;
	}

	dn = rc_sum/num_sites;
	lamda = 5.052*pow((2*dn/M_PI),2.0);
	s_gamma = 0.2;
	for (auto j=0; j<WGD->ny; j++)
	{
		for (auto i=0; i<WGD->nx; i++)
		{
			sum_wm = 0.0;
			for (auto ii=0; ii<num_sites; ii++)
			{
				wm[ii][i][j] = exp((-1/lamda)*pow(WID->metParams->sensors[ii]->site_xcoord-x[i],2.0)-(1/lamda)*pow(WID->metParams->sensors[ii]->site_ycoord-y[j],2.0));
				wms[ii][i][j] = exp((-1/(s_gamma*lamda))*pow(WID->metParams->sensors[ii]->site_xcoord-x[i],2.0)-(1/(s_gamma*lamda))*
									pow(WID->metParams->sensors[ii]->site_ycoord-y[j],2.0));
				sum_wm += wm[ii][i][j];
			}
			if (sum_wm == 0)
			{
				for (auto ii = 0; ii<num_sites; ii++)
				{
					wm[ii][i][j] = 1e-20;
				}
			}
		}
	}

	for (auto k=1; k<WGD->nz; k++)
	{
		for (auto j=0; j<WGD->ny; j++)
		{
			for (auto i=0; i<WGD->nx; i++)
			{
				sum_wu = 0.0;
				sum_wv = 0.0;
				sum_wm = 0.0;
				for (auto ii=0; ii<num_sites; ii++)
				{
					sum_wu += wm[ii][i][j]*u_prof[ii][k];
					sum_wv += wm[ii][i][j]*v_prof[ii][k];
					sum_wm += wm[ii][i][j];
				}

				icell_face = i + j*WGD->nx + k*WGD->nx*WGD->ny;
				WGD->u0[icell_face] = sum_wu/sum_wm;
				WGD->v0[icell_face] = sum_wv/sum_wm;
				WGD->w0[icell_face] = 0.0;
			}
		}



		for (auto ii=0; ii<num_sites; ii++)
		{
			if(WID->metParams->sensors[ii]->site_xcoord>0 && WID->metParams->sensors[ii]->site_xcoord < (WGD->nx-1)*WGD->dx && WID->metParams->sensors[ii]->site_ycoord > 0 && WID->metParams->sensors[ii]->site_ycoord<(WGD->ny-1)*WGD->dy)
			{
				for (auto j=0; j<WGD->ny; j++)
				{
					if (y[j]<WID->metParams->sensors[ii]->site_ycoord)
					{
						jwork = j;
					}
				}

				for (auto i=0; i<WGD->nx; i++)
				{
					if (x[i]<WID->metParams->sensors[ii]->site_xcoord)
					{
						iwork = i;
					}
				}

				dxx = WID->metParams->sensors[ii]->site_xcoord-x[iwork];
				dyy = WID->metParams->sensors[ii]->site_ycoord-y[jwork];
				int index_work = iwork+jwork*WGD->nx+k*WGD->nx*WGD->ny;
				u12 = (1-(dxx/WGD->dx))*WGD->u0[index_work+WGD->nx]+(dxx/WGD->dx)*WGD->u0[index_work+1+WGD->nx];
				u34 = (1-(dxx/WGD->dx))*WGD->u0[index_work]+(dxx/WGD->dx)*WGD->u0[index_work+1];
				u0_int[ii] = (dyy/WGD->dy)*u12+(1-(dyy/WGD->dy))*u34;



				v12 = (1-(dxx/WGD->dx))*WGD->v0[index_work+WGD->nx]+(dxx/WGD->dx)*WGD->v0[index_work+1+WGD->nx];
				v34 = (1-(dxx/WGD->dx))*WGD->v0[index_work]+(dxx/WGD->dx)*WGD->v0[index_work+1];
				v0_int[ii] = (dyy/WGD->dy)*v12+(1-(dyy/WGD->dy))*v34;

			}
			else
			{
				u0_int[ii] = u_prof[ii][k];
				v0_int[ii] = v_prof[ii][k];
			}
		}

		for (auto j=0; j<WGD->ny; j++)
		{
			for (auto i=0; i<WGD->nx; i++)
			{
				sum_wu = 0.0;
				sum_wv = 0.0;
				sum_wm = 0.0;
				for (auto ii=0; ii<num_sites; ii++)
				{
					sum_wu += wm[ii][i][j]*(u_prof[ii][k]-u0_int[ii]);
					sum_wv += wm[ii][i][j]*(v_prof[ii][k]-v0_int[ii]);
					sum_wm += wm[ii][i][j];
				}

				if (sum_wm != 0)
				{
					icell_face = i + j*WGD->nx + k*WGD->nx*WGD->ny;
					WGD->u0[icell_face] = WGD->u0[icell_face]+sum_wu/sum_wm;
					WGD->v0[icell_face] = WGD->v0[icell_face]+sum_wv/sum_wm;
				}
			}
		}
	}

}



void Sensor::UTMConverter (float rlon, float rlat, float rx, float ry, int UTM_PROJECTION_ZONE, int iway)
{


/*

                S p e c f e m 3 D  V e r s i o n  2 . 1
                ---------------------------------------

           Main authors: Dimitri Komatitsch and Jeroen Tromp
     Princeton University, USA and CNRS / INRIA / University of Pau
  (c) Princeton University / California Institute of Technology and CNRS / INRIA / University of Pau
                              July 2012

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) aWGD->ny later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT AWGD->ny WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

/*
  UTM (Universal Transverse Mercator) projection from the USGS
*/



/*
convert geodetic longitude and latitude to UTM, and back
use iway = ILONGLAT2UTM for long/lat to UTM, IUTM2LONGLAT for UTM to lat/long
a list of UTM zones of the world is available at www.dmap.co.uk/utmworld.htm
*/


/*
      CAMx v2.03

      UTM_GEO performs UTM to geodetic (long/lat) translation, and back.

      This is a Fortran version of the BASIC program "Transverse Mercator
      Conversion", Copyright 1986, Norman J. Berls (Stefan Musarra, 2/94)
      Based on algorithm taken from "Map Projections Used by the USGS"
      by John P. SWGD->nyder, Geological Survey Bulletin 1532, USDI.

      Input/Output arguments:

         rlon                  Longitude (deg, negative for West)
         rlat                  Latitude (deg)
         rx                    UTM easting (m)
         ry                    UTM northing (m)
         UTM_PROJECTION_ZONE  UTM zone
         iway                  Conversion type
                               ILONGLAT2UTM = geodetic to UTM
                               IUTM2LONGLAT = UTM to geodetic
*/


  int ILONGLAT2UTM = 0, IUTM2LONGLAT = 1;
  float PI = 3.141592653589793;
  float degrad = PI/180.0;
  float raddeg = 180.0/PI;
  float semimaj = 6378206.40;
  float semimin = 6356583.80;
  float scfa = 0.99960;

/*
  some extracts about UTM:

  There are 60 longitudinal projection zones numbered 1 to 60 starting at 180Â°W.
  Each of these zones is 6 degrees wide, apart from a few exceptions around Norway and Svalbard.
  There are 20 latitudinal zones spanning the latitudes 80Â°S to 84Â°N and denoted
  by the letters C to X, ommitting the letter O.
  Each of these is 8 degrees south-north, apart from zone X which is 12 degrees south-north.

  To change the UTM zone and the hemisphere in which the
  calculations are carried out, need to change the fortran code and recompile. The UTM zone is described
  actually by the central meridian of that zone, i.e. the longitude at the midpoint of the zone, 3 degrees
  from either zone boundary.
  To change hemisphere need to change the "north" variable:
  - north=0 for northern hemisphere and
  - north=10000000 (10000km) for southern hemisphere. values must be in metres i.e. north=10000000.

  Note that the UTM grids are actually Mercators which
  employ the standard UTM scale factor 0.9996 and set the
  Easting Origin to 500,000;
  the Northing origin in the southern
  hemisphere is kept at 0 rather than set to 10,000,000
  and this gives a uniform scale across the equator if the
  normal convention of selecting the Base Latitude (origin)
  at the equator (0 deg.) is followed.  Northings are
  positive in the northern hemisphere and negative in the
  southern hemisphere.
  */

  float north = 0.0;
  float east = 500000.0;

  float e2,e4,e6,ep2,xx,yy,dlat,dlon,zone,cm,cmr,delam;
  float f1,f2,f3,f4,rm,rn,t,c,a,e1,u,rlat1,dlat1,c1,t1,rn1,r1,d;
  float rx_save,ry_save,rlon_save,rlat_save;

  // save original parameters
  rlon_save = rlon;
  rlat_save = rlat;
  rx_save = rx;
  ry_save = ry;

  xx = 0.0;
  yy = 0.0;
  dlat = 0.0;
  dlon = 0.0;

  // define parameters of reference ellipsoid
  e2 = 1.0-pow((semimin/semimaj),2.0);
  e4 = pow(e2,2.0);
  e6 = e2*e4;
  ep2 = e2/(1.0-e2);

  if (iway == IUTM2LONGLAT)
  {
    xx = rx;
    yy = ry;
  }
  else
  {
    dlon = rlon;
    dlat = rlat;
  }

  // Set Zone parameters

  zone = UTM_PROJECTION_ZONE;
  // sets central meridian for this zone
  cm = zone*6.0 - 183.0;
  cmr = cm*degrad;

  // Lat/Lon to UTM conversion

  if (iway == ILONGLAT2UTM)
  {
    rlon = degrad*dlon;
    rlat = degrad*dlat;

    delam = dlon - cm;
    if (delam < -180.0)
    {
      delam = delam + 360.0;
    }
    if (delam > 180.0)
    {
      delam = delam - 360.0;
    }
    delam = delam*degrad;

    f1 = (1.0 - (e2/4.0) - 3.0*(e4/64.0) - 5.0*(e6/256))*rlat;
    f2 = 3.0*(e2/8.0) + 3.0*(e4/32.0) + 45.0*(e6/1024.0);
    f2 = f2*sin(2.0*rlat);
    f3 = 15.0*(e4/256.0)*45.0*(e6/1024.0);
    f3 = f3*sin(4.0*rlat);
    f4 = 35.0*(e6/3072.0);
    f4 = f4*sin(6.0*rlat);
    rm = semimaj*(f1 - f2 + f3 - f4);
    if (dlat == 90.0 || dlat == -90.0)
    {
      xx = 0.0;
      yy = scfa*rm;
    }
    else
    {
      rn = semimaj/sqrt(1.0 - e2*pow(sin(rlat),2.0));
      t = pow(tan(rlat),2.0);
      c = ep2*pow(cos(rlat),2.0);
      a = cos(rlat)*delam;

      f1 = (1.0 - t + c)*pow(a,3.0)/6.0;
      f2 = 5.0 - 18.0*t + pow(t,2.0) + 72.0*c - 58.0*ep2;
      f2 = f2*pow(a,5.0)/120.0;
      xx = scfa*rn*(a + f1 + f2);
      f1 = pow(a,2.0)/2.0;
      f2 = 5.0 - t + 9.0*c + 4.0*pow(c,2.0);
      f2 = f2*pow(a,4.0)/24.0;
      f3 = 61.0 - 58.0*t + pow(t,2.0) + 600.0*c - 330.0*ep2;
      f3 = f3*pow(a,6.0)/720.0;
      yy = scfa*(rm + rn*tan(rlat)*(f1 + f2 + f3));
    }
    xx = xx + east;
    yy = yy + north;
  }

  // UTM to Lat/Lon conversion

  else
  {
    xx = xx - east;
    yy = yy - north;
    e1 = sqrt(1.0 - e2);
    e1 = (1.0 - e1)/(1.0 + e1);
    rm = yy/scfa;
    u = 1.0 - (e2/4.0) - 3.0*(e4/64.0) - 5.0*(e6/256.0);
    u = rm/(semimaj*u);

    f1 = 3.0*(e1/2.0) - 27.0*pow(e1,3.0)/32.0;
    f1 = f1*sin(2.0*u);
    f2 = (21.0*pow(e1,2.0)/16.0) - 55.0*pow(e1,4.0)/32.0;
    f2 = f2*sin(4.0*u);
    f3 = 151.0*pow(e1,3.0)/96.0;
    f3 = f3*sin(6.0*u);
    rlat1 = u + f1 + f2 + f3;
    dlat1 = rlat1*raddeg;
    if (dlat1 >= 90.0 || dlat1 <= -90.0)
    {
      dlat1 = std::min(dlat1,90.0f) ;
      dlat1 = std::max(dlat1,-90.0f);
      dlon = cm;
    }
    else
    {
      c1 = ep2*pow(cos(rlat1),2.0);
      t1 = pow(tan(rlat1),2.0);
      f1 = 1.0 - e2*pow(sin(rlat1),2.0);
      rn1 = semimaj/sqrt(f1);
      r1 = semimaj*(1.0 - e2)/sqrt(pow(f1,3.0));
      d = xx/(rn1*scfa);

      f1 = rn1*tan(rlat1)/r1;
      f2 = pow(d,2.0)/2.0;
      f3 = 5.0*3.0*t1 + 10.0*c1 - 4.0*pow(c1,2.0) - 9.0*ep2;
      f3 = f3*pow(d,2.0)*pow(d,2.0)/24.0;
      f4 = 61.0 + 90.0*t1 + 298.0*c1 + 45.0*pow(t1,2.0) - 252.0*ep2 - 3.0*pow(c1,2.0);
      f4 = f4*pow(pow(d,2.0),3.0)/720.0;
      rlat = rlat1 - f1*(f2 - f3 + f4);
      dlat = rlat*raddeg;

      f1 = 1.0 + 2.0*t1 + c1;
      f1 = f1*pow(d,2.0)*d/6.0;
      f2 = 5.0 - 2.0*c1 + 28.0*t1 - 3.0*pow(c1,2.0) + 8.0*ep2 + 24.0*pow(t1,2.0);
      f2 = f2*pow(pow(d,2.0),2.0)*d/120.0;
      rlon = cmr + (d - f1 + f2)/cos(rlat1);
      dlon = rlon*raddeg;
      if (dlon < -180.0)
      {
        dlon = dlon + 360.0;
      }
      if (dlon > 180.0)
      {
        dlon = dlon - 360.0;
      }
    }
  }

  if (iway == IUTM2LONGLAT)
  {
    rlon = dlon;
    rlat = dlat;
    rx = rx_save;
    ry = ry_save;
  }
  else
  {
    rx = xx;
    ry = yy;
    rlon = rlon_save;
    rlat = rlat_save;
  }

}


void Sensor::getConvergence(float lon, float lat, int site_UTM_zone, float convergence)
{

	float temp_lon;
	temp_lon = (6.0*site_UTM_zone) - 183.0 -lon;
	convergence = atan(atan(temp_lon*M_PI/180.0)*asin(lat*M_PI/180.0))*(180.0/M_PI);

}
