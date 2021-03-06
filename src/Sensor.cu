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


#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <vector>
#include <chrono>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include "Sensor.h"

#include "WINDSInputData.h"
#include "WINDSGeneralData.h"

#define BLOCKSIZE 1024
#define cudaCheck(x) _cudaCheck(x, #x ,__FILE__, __LINE__)

template<typename T>
void Sensor::_cudaCheck(T e, const char* func, const char* call, const int line){
    if(e != cudaSuccess){
        printf("\"%s\" at %d in %s\n\treturned %d\n-> %s\n", func, line, call, (int)e, cudaGetErrorString(e));
        exit(EXIT_FAILURE);
    }
}

__global__ void Calculatewm (float *d_wm, float *d_wms, float *d_sum_wm, float *d_site_xcoord, float *d_site_ycoord,
                              float *d_x, float *d_y, float lamda, float s_gamma, int num_sites, int nx, int ny, int nz)
{

  int ii = blockDim.x*blockIdx.x+threadIdx.x;
  int j = ii/((nx)*num_sites);
  int i = (ii - j*(nx)*num_sites)/num_sites;
  int site_id = ii - j*(nx)*num_sites - i*num_sites;

  if( (i<nx) && (j<ny) && (site_id<num_sites) && (i>=0) && (j>=0) && (site_id>=0) )
  {
    d_wm[ii] = exp((-1/lamda)*pow(d_site_xcoord[site_id]-d_x[i],2.0)-(1/lamda)*pow(d_site_ycoord[site_id]-d_y[j],2.0));
    d_wms[ii] = exp((-1/(s_gamma*lamda))*pow(d_site_xcoord[site_id]-d_x[i],2.0)-(1/(s_gamma*lamda))*pow(d_site_ycoord[site_id]-d_y[j],2.0));

  }
  __syncthreads();

  if( (i<nx) && (j<ny) && (site_id<num_sites) && (i>=0) && (j>=0) && (site_id>=0) )
  {
    if (site_id == 0)
    {
      for (int id = 0; id < num_sites; id++)
      {
        d_sum_wm[i+j*nx] = d_sum_wm[i+j*nx] + d_wm[ii+id];
      }
    }
  }

  if( (i<nx) && (j<ny) && (site_id<num_sites) && (i>=0) && (j>=0) && (site_id>=0) )
  {
    if (d_sum_wm[i+j*nx] == 0)
    {
      d_wm[ii] = 1e-20;
    }
  }
}


__global__ void CalculateInitialWind (float *d_wm, float *d_sum_wm, float *d_sum_wu, float *d_sum_wv, float *d_u0,
                                      float *d_v0, float *d_w0, float *d_u_prof, float *d_v_prof, int num_sites, int nx, int ny, int nz)
{

  int ii = blockDim.x*blockIdx.x+threadIdx.x;
  int j = ii/(nx);
  int i = (ii - j*nx);

  if( (i<nx) && (j<ny) && (i>=0) && (j>=0) )
  {
    for (int k = 1; k < nz; k++)
    {
      int idx = i+j*nx+k*nx*ny;
      d_sum_wu[idx] = 0;
      d_sum_wv[idx] = 0;
      d_sum_wm[i+j*nx] = 0;
      for (int id = 0; id < num_sites; id++)
      {
        d_sum_wu[idx] = d_sum_wu[idx] + d_wm[id+i*num_sites+j*num_sites*nx]*d_u_prof[k + id*nz];
        d_sum_wv[idx] = d_sum_wv[idx] + d_wm[id+i*num_sites+j*num_sites*nx]*d_v_prof[k + id*nz];
        d_sum_wm[i+j*nx] = d_sum_wm[i+j*nx] + d_wm[id+i*num_sites+j*num_sites*nx];
      }
      int icell_face = i + j*nx + k*nx*ny;
      d_u0[icell_face] = d_sum_wu[idx]/d_sum_wm[i+j*nx];
      d_v0[icell_face] = d_sum_wv[idx]/d_sum_wm[i+j*nx];
      d_w0[icell_face] = 0.0;
    }

  }

}


__global__ void CalculateInit (float *d_site_xcoord, float *d_site_ycoord, float *d_x, float *d_y, float *d_dxx, float *d_dyy,
                                int *d_iwork, int *d_jwork, float *d_u12, float *d_u34, float *d_v12, float *d_v34, float *d_u0_int,
                                float *d_v0_int, float *d_u0, float *d_v0, float *d_u_prof, float *d_v_prof, int num_sites,
                                float dx, float dy, int nx, int ny, int nz)
{
  int ii = blockDim.x*blockIdx.x+threadIdx.x;
  if (ii < num_sites)
  {
    if(d_site_xcoord[ii] > 0 && d_site_xcoord[ii] < (nx-1)*dx && d_site_ycoord[ii] > 0 && d_site_ycoord[ii] < (ny-1)*dy)
    {

      for (int j = 0; j < ny; j++)
      {
        if (d_y[j] < d_site_ycoord[ii])
        {
          d_jwork[ii] = j;
        }
      }
      for (int i = 0; i < nx; i++)
      {

        if (d_x[i] < d_site_xcoord[ii])
        {
          d_iwork[ii] = i;
        }
      }

      d_dxx[ii] = d_site_xcoord[ii]-d_x[d_iwork[ii]];
      d_dyy[ii] = d_site_ycoord[ii]-d_y[d_jwork[ii]];

      for (int k = 1; k < nz; k++)
    	{
        int id = k + ii*nz;
				d_u12[id] = (1-(d_dxx[ii]/dx))*d_u0[d_iwork[ii]+d_jwork[ii]*nx+k*nx*ny+nx]+(d_dxx[ii]/dx)*d_u0[d_iwork[ii]+d_jwork[ii]*nx+k*nx*ny+1+nx];
        d_u34[id] = (1-(d_dxx[ii]/dx))*d_u0[d_iwork[ii]+d_jwork[ii]*nx+k*nx*ny]+(d_dxx[ii]/dx)*d_u0[d_iwork[ii]+d_jwork[ii]*nx+k*nx*ny+1];
        d_u0_int[ii] = (d_dyy[ii]/dy)*d_u12[id]+(1-(d_dyy[ii]/dy))*d_u34[id];

				d_v12[id] = (1-(d_dxx[ii]/dx))*d_v0[d_iwork[ii]+d_jwork[ii]*nx+k*nx*ny+nx]+(d_dxx[ii]/dx)*d_v0[d_iwork[ii]+d_jwork[ii]*nx+k*nx*ny+1+nx];
				d_v34[id] = (1-(d_dxx[ii]/dx))*d_v0[d_iwork[ii]+d_jwork[ii]*nx+k*nx*ny]+(d_dxx[ii]/dx)*d_v0[d_iwork[ii]+d_jwork[ii]*nx+k*nx*ny+1];
				d_v0_int[ii] = (d_dyy[ii]/dy)*d_v12[id]+(1-(d_dyy[ii]/dy))*d_v34[id];


        d_u_prof [id] = d_u_prof [id] - d_u0_int[ii];
        d_v_prof [id] = d_v_prof [id] - d_v0_int[ii];

      }

    }
    else
    {
      for (int k = 1; k < nz; k++)
    	{
        int id = k + ii*nz;
        d_u0_int[ii] = d_u_prof[id];
        d_v0_int[ii] = d_v_prof[id];

        d_u_prof [id] = d_u_prof [id] - d_u0_int[ii];
        d_v_prof [id] = d_v_prof [id] - d_v0_int[ii];
      }
    }
  }

}

__global__ void CorrectInitialWind (float *d_wm, float *d_sum_wm, float *d_sum_wu, float *d_sum_wv, float *d_u0,
                                      float *d_v0, float *d_w0, float *d_u_prof, float *d_v_prof, float *d_u0_int, float *d_v0_int,
                                      int num_sites, int nx, int ny, int nz)
{

  int ii = blockDim.x*blockIdx.x+threadIdx.x;
  int j = ii/(nx);
  int i = (ii - j*nx);

  if( (i<nx) && (j<ny) && (i>=0) && (j>=0) )
  {
    for (int k = 1; k < nz; k++)
    {
      int idx = i+j*nx+k*nx*ny;
      d_sum_wu[idx] = 0;
      d_sum_wv[idx] = 0;
      d_sum_wm[i+j*nx] = 0;
      for (int id = 0; id < num_sites; id++)
      {
        d_sum_wu[idx] = d_sum_wu[idx] + d_wm[id+i*num_sites+j*num_sites*nx]* d_u_prof[k + id*nz];
        d_sum_wv[idx] = d_sum_wv[idx] + d_wm[id+i*num_sites+j*num_sites*nx]* d_v_prof[k + id*nz];
        d_sum_wm[i+j*nx] = d_sum_wm[i+j*nx] + d_wm[id+i*num_sites+j*num_sites*nx];

      }


      if (d_sum_wm[i+j*nx] != 0.0)
      {
        int icell_face = i + j*nx + k*nx*ny;
        d_u0[icell_face] = d_u0[icell_face] + d_sum_wu[idx]/d_sum_wm[i+j*nx];
        d_v0[icell_face] = d_v0[icell_face] + d_sum_wv[idx]/d_sum_wm[i+j*nx];
      }

    }

  }

}



__global__ void BarnesScheme (float *d_u_prof, float *d_v_prof, float *d_wm, float *d_wms, float *d_u0_int, float *d_v0_int,
                              float *d_x, float *d_y, float *d_site_xcoord, float *d_site_ycoord, float *d_sum_wm, float *d_sum_wu,
                              float *d_sum_wv, float *d_u0, float *d_v0, float *d_w0, int *d_iwork, int *d_jwork, float *d_dxx,
                              float *d_dyy, float *d_u12, float *d_u34, float *d_v12, float *d_v34, int num_sites, int nx, int ny,
                              int nz, float dx, float dy)
{
  float rc_sum, rc_val, xc, yc, rc;
	float dn, lamda, s_gamma;
  rc_sum = 0.0;
	for (int i = 0; i < num_sites; i++)
	{
		rc_val = 1000000.0;
		for (int ii = 0; ii < num_sites; ii++)
		{
			xc = d_site_xcoord [ii] - d_site_xcoord [i];
			yc = d_site_ycoord [ii] - d_site_ycoord [i];
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

  dim3 numberOfThreadsPerBlock(BLOCKSIZE,1,1);
  dim3 numberOfBlocks(ceil((num_sites*nx*ny)/(float) (BLOCKSIZE)),1,1);

  Calculatewm<<<numberOfBlocks,numberOfThreadsPerBlock>>>(d_wm, d_wms, d_sum_wm, d_site_xcoord, d_site_ycoord, d_x, d_y, lamda, s_gamma, num_sites, nx, ny, nz);
  cudaDeviceSynchronize();

  dim3 numberOfThreadsPerBlock1(BLOCKSIZE,1,1);
  dim3 numberOfBlocks1(ceil((nx*ny)/(float) (BLOCKSIZE)),1,1);

  CalculateInitialWind<<<numberOfBlocks1,numberOfThreadsPerBlock1>>>(d_wm, d_sum_wm, d_sum_wu, d_sum_wv, d_u0, d_v0, d_w0, d_u_prof, d_v_prof, num_sites, nx, ny, nz);
  cudaDeviceSynchronize();

  dim3 numberOfThreadsPerBlock2(BLOCKSIZE,1,1);
  dim3 numberOfBlocks2(ceil((num_sites)/(float) (BLOCKSIZE)),1,1);

  CalculateInit<<<numberOfBlocks2,numberOfThreadsPerBlock2>>>(d_site_xcoord, d_site_ycoord, d_x, d_y, d_dxx, d_dyy, d_iwork, d_jwork, d_u12, d_u34, d_v12, d_v34, d_u0_int, d_v0_int, d_u0, d_v0, d_u_prof, d_v_prof,num_sites, dx, dy, nx, ny, nz);
  cudaDeviceSynchronize();

  CorrectInitialWind<<<numberOfBlocks1,numberOfThreadsPerBlock1>>>(d_wm, d_sum_wm, d_sum_wu, d_sum_wv, d_u0, d_v0, d_w0, d_u_prof, d_v_prof, d_u0_int, d_v0_int, num_sites, nx, ny, nz);
  cudaDeviceSynchronize();

}


void Sensor::BarnesInterpolationGPU(const WINDSInputData *WID, WINDSGeneralData *WGD, std::vector<std::vector<float>> u_prof, std::vector<std::vector<float>> v_prof)
{

  int num_sites = WID->metParams->sensors.size();

  std::vector<float> u_prof_1d, v_prof_1d, wm, wms, u0_int, v0_int;
  std::vector<float> site_xcoord, site_ycoord, sum_wm, sum_wu, sum_wv;
  std::vector<float> dxx, dyy, u12, u34, v12, v34;
  std::vector<int> iwork, jwork;
  u_prof_1d.resize(num_sites*WGD->nz, 0.0);
  v_prof_1d.resize(num_sites*WGD->nz, 0.0);
  wm.resize(num_sites*WGD->nx*WGD->ny, 0.0);
  wms.resize(num_sites*WGD->nx*WGD->ny, 0.0);
  u0_int.resize(num_sites, 0.0);
  v0_int.resize(num_sites, 0.0);
  sum_wm.resize(WGD->nx*WGD->ny, 0.0);
  sum_wu.resize(WGD->nx*WGD->ny*WGD->nz, 0.0);
  sum_wv.resize(WGD->nx*WGD->ny*WGD->nz, 0.0);
  site_xcoord.resize(num_sites, 0.0);
  site_ycoord.resize(num_sites, 0.0);
  iwork.resize(num_sites, 0);
  jwork.resize(num_sites, 0);
  dxx.resize(num_sites, 0.0);
  dyy.resize(num_sites, 0.0);
  u12.resize(num_sites*WGD->nz, 0.0);
  u34.resize(num_sites*WGD->nz, 0.0);
  v12.resize(num_sites*WGD->nz, 0.0);
  v34.resize(num_sites*WGD->nz, 0.0);

  for (auto i = 0 ; i < num_sites; i++)
	{
    for (auto k = 0; k < WGD->nz; k++)
    {
      int id = k + i*WGD->nz;
      u_prof_1d [id] = u_prof[i][k];
      v_prof_1d [id] = v_prof[i][k];
    }
  }

  for (auto i = 0 ; i < num_sites; i++)
	{
    site_xcoord[i] = WID->metParams->sensors[i]->site_xcoord;
    site_ycoord[i] = WID->metParams->sensors[i]->site_ycoord;
  }


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

  float *d_u_prof, *d_v_prof, *d_wm, *d_wms, *d_u0_int, *d_v0_int;
  float *d_x, *d_y, *d_site_xcoord, *d_site_ycoord, *d_sum_wm, *d_sum_wu, *d_sum_wv;
  float *d_u0, *d_v0, *d_w0;
  float *d_dxx, *d_dyy, *d_u12, *d_u34, *d_v12, *d_v34;
  int *d_iwork, *d_jwork;

  cudaMalloc((void **) &d_u_prof, num_sites*WGD->nz * sizeof(float));
  cudaMalloc((void **) &d_v_prof, num_sites*WGD->nz * sizeof(float));
  cudaMalloc((void **) &d_wm, num_sites*WGD->nx*WGD->ny * sizeof(float));
  cudaMalloc((void **) &d_wms, num_sites*WGD->nx*WGD->ny * sizeof(float));
  cudaMalloc((void **) &d_u0_int, num_sites * sizeof(float));
  cudaMalloc((void **) &d_v0_int, num_sites * sizeof(float));
  cudaMalloc((void **) &d_sum_wm, WGD->nx*WGD->ny * sizeof(float));
  cudaMalloc((void **) &d_sum_wu, WGD->nx*WGD->ny*WGD->nz * sizeof(float));
  cudaMalloc((void **) &d_sum_wv, WGD->nx*WGD->ny*WGD->nz * sizeof(float));
  cudaMalloc((void **) &d_site_xcoord, num_sites * sizeof(float));
  cudaMalloc((void **) &d_site_ycoord, num_sites * sizeof(float));
  cudaMalloc((void **) &d_x, WGD->nx * sizeof(float));
  cudaMalloc((void **) &d_y, WGD->ny * sizeof(float));
  cudaMalloc((void **) &d_u0,WGD->numcell_face*sizeof(float));
  cudaMalloc((void **) &d_v0,WGD->numcell_face*sizeof(float));
  cudaMalloc((void **) &d_w0,WGD->numcell_face*sizeof(float));
  cudaMalloc((void **) &d_iwork, num_sites * sizeof(int));
  cudaMalloc((void **) &d_jwork, num_sites * sizeof(int));
  cudaMalloc((void **) &d_dxx, num_sites * sizeof(float));
  cudaMalloc((void **) &d_dyy, num_sites * sizeof(float));
  cudaMalloc((void **) &d_u12, num_sites*WGD->nz * sizeof(float));
  cudaMalloc((void **) &d_u34, num_sites*WGD->nz * sizeof(float));
  cudaMalloc((void **) &d_v12, num_sites*WGD->nz * sizeof(float));
  cudaMalloc((void **) &d_v34, num_sites*WGD->nz * sizeof(float));

  cudaMemcpy(d_u_prof, u_prof_1d.data(), num_sites*WGD->nz*sizeof(int),cudaMemcpyHostToDevice);
  cudaMemcpy(d_v_prof, v_prof_1d.data(), num_sites*WGD->nz*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_wm, wm.data(), num_sites*WGD->nx*WGD->ny*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_wms, wms.data(), num_sites*WGD->nx*WGD->ny*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_u0_int, u0_int.data(), num_sites * sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_v0_int, v0_int.data() , num_sites * sizeof(float) , cudaMemcpyHostToDevice);
  cudaMemcpy(d_sum_wm, sum_wm.data() , WGD->nx*WGD->ny * sizeof(float) , cudaMemcpyHostToDevice);
  cudaMemcpy(d_sum_wu, sum_wu.data() , WGD->nx*WGD->ny*WGD->nz * sizeof(float) , cudaMemcpyHostToDevice);
  cudaMemcpy(d_sum_wv, sum_wv.data() , WGD->nx*WGD->ny*WGD->nz * sizeof(float) , cudaMemcpyHostToDevice);
  cudaMemcpy(d_site_xcoord, site_xcoord.data(), num_sites*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_site_ycoord, site_ycoord.data(), num_sites*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_x , x.data() , WGD->nx * sizeof(float) , cudaMemcpyHostToDevice);
  cudaMemcpy(d_y , y.data() , WGD->ny * sizeof(float) , cudaMemcpyHostToDevice);
  cudaMemcpy(d_u0, WGD->u0.data(),WGD->numcell_face*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_v0, WGD->v0.data(),WGD->numcell_face*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_w0, WGD->w0.data(),WGD->numcell_face*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_iwork, iwork.data(), num_sites*sizeof(int),cudaMemcpyHostToDevice);
  cudaMemcpy(d_jwork, jwork.data(), num_sites*sizeof(int),cudaMemcpyHostToDevice);
  cudaMemcpy(d_dxx, dxx.data(), num_sites*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_dyy, dyy.data(), num_sites*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_u12, u12.data(), num_sites*WGD->nz*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_u34, u34.data(), num_sites*WGD->nz*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_v12, v12.data(), num_sites*WGD->nz*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(d_v34, v34.data(), num_sites*WGD->nz*sizeof(float),cudaMemcpyHostToDevice);

  BarnesScheme<<<1,1>>>(d_u_prof, d_v_prof, d_wm, d_wms, d_u0_int, d_v0_int, d_x, d_y, d_site_xcoord, d_site_ycoord, d_sum_wm, d_sum_wu,d_sum_wv, d_u0, d_v0, d_w0, d_iwork, d_jwork, d_dxx, d_dyy, d_u12, d_u34, d_v12, d_v34, num_sites,WGD->nx, WGD->ny,WGD->nz, WGD->dx, WGD->dy);
  cudaCheck(cudaGetLastError());

  cudaMemcpy(WGD->u0.data(),d_u0,WGD->numcell_face*sizeof(float),cudaMemcpyDeviceToHost);
  cudaMemcpy(WGD->v0.data(),d_v0,WGD->numcell_face*sizeof(float),cudaMemcpyDeviceToHost);
  cudaMemcpy(WGD->w0.data(),d_w0,WGD->numcell_face*sizeof(float),cudaMemcpyDeviceToHost);

  cudaFree (d_u_prof);
  cudaFree (d_v_prof);
  cudaFree (d_wm);
  cudaFree (d_wms);
  cudaFree (d_u0_int);
  cudaFree (d_v0_int);
  cudaFree (d_site_xcoord);
  cudaFree (d_site_ycoord);
  cudaFree (d_x);
  cudaFree (d_y);
  cudaFree (d_u0);
  cudaFree (d_v0);
  cudaFree (d_w0);
  cudaFree (d_sum_wm);
  cudaFree (d_sum_wu);
  cudaFree (d_sum_wv);
  cudaFree (d_iwork);
  cudaFree (d_jwork);
  cudaFree (d_dxx);
  cudaFree (d_dyy);
  cudaFree (d_u12);
  cudaFree (d_u34);
  cudaFree (d_v12);
  cudaFree (d_v34);


}
