#include "GlobalMemory.h"

using namespace std::chrono;
using namespace std;
using std::ofstream;
using std::ifstream;
using std::istringstream;
using std::string;
using std::cerr;
using std::endl;
using std::vector;
using std::cout;
using std::to_string;

#define BLOCKSIZE 1024
#define cudaCheck(x) _cudaCheck(x, #x ,__FILE__, __LINE__)


template<typename T>
void GlobalMemory::_cudaCheck(T e, const char* func, const char* call, const int line){
    if(e != cudaSuccess){
        printf("\"%s\" at %d in %s\n\treturned %d\n-> %s\n", func, line, call, (int)e, cudaGetErrorString(e));
        exit(EXIT_FAILURE);
    }
}

/// Divergence CUDA Kernel.
/// The divergence kernel ...
///
__global__ void divergenceGlobal(float *d_u0, float *d_v0, float *d_w0, float *d_R, float *d_e, float *d_f, float *d_g,
						float *d_h, float *d_m, float *d_n, int alpha1, int  nx, int  ny, int nz,float dx,float dy,float *d_dz_array)
{

    int icell_cent = blockDim.x*blockIdx.x+threadIdx.x;
    int k = icell_cent/((nx-1)*(ny-1));
    int j = (icell_cent - k*(nx-1)*(ny-1))/(nx-1);
    int i = icell_cent - k*(nx-1)*(ny-1) - j*(nx-1);
    int icell_face = i + j*nx + k*nx*ny;

    // Would be nice to figure out how to not have this branch check...
    if( (i<nx-1) && (j<ny-1) && (k<nz-2) && (i>=0) && (j>=0) && (k>0) )
    {

        // Divergence equation
        d_R[icell_cent] = (-2*pow(alpha1, 2.0))*((( d_e[icell_cent] * d_u0[icell_face+1]       - d_f[icell_cent] * d_u0[icell_face]) * dx ) +
                                               (( d_g[icell_cent] * d_v0[icell_face + nx]    - d_h[icell_cent] * d_v0[icell_face]) * dy ) +
                                               ( d_m[icell_cent] * d_dz_array[k]*0.5*(d_dz_array[k]+d_dz_array[k+1]) * d_w0[icell_face + nx*ny]
                                                - d_n[icell_cent] * d_w0[icell_face] * d_dz_array[k]*0.5*(d_dz_array[k]+d_dz_array[k-1]) ));

    }
}


/// SOR RedBlack Kernel.
///
///
__global__ void SOR_RB_Global(float *d_lambda, int nx, int ny, int nz, float omega, float  A, float  B, float *d_e,
						float *d_f, float *d_g, float *d_h, float *d_m, float *d_n, float *d_R, int offset)
{
    int icell_cent = blockDim.x*blockIdx.x+threadIdx.x;
    int k = icell_cent/((nx-1)*(ny-1));
    int j = (icell_cent - k*(nx-1)*(ny-1))/(nx-1);
    int i = icell_cent - k*(nx-1)*(ny-1) - j*(nx-1);

    if ( (i > 0) && (i < nx-2) && (j > 0) && (j < ny-2) && (k < nz-2) && (k > 0) && ((i+j+k)%2) == offset ){

        d_lambda[icell_cent] = (omega / ( d_e[icell_cent] + d_f[icell_cent] + d_g[icell_cent] +
                                          d_h[icell_cent] + d_m[icell_cent] + d_n[icell_cent])) *
            ( d_e[icell_cent] * d_lambda[icell_cent+1]               + d_f[icell_cent] * d_lambda[icell_cent-1] +
              d_g[icell_cent] * d_lambda[icell_cent + (nx-1)]        + d_h[icell_cent] * d_lambda[icell_cent - (nx-1)] +
              d_m[icell_cent] * d_lambda[icell_cent + (nx-1)*(ny-1)] +
              d_n[icell_cent] * d_lambda[icell_cent - (nx-1)*(ny-1)] - d_R[icell_cent] ) +
            (1.0 - omega) * d_lambda[icell_cent];    /// SOR formulation
    }
}


// Euler Final Velocity kernel
__global__ void finalVelocityGlobal(float *d_u0, float *d_v0, float *d_w0, float *d_lambda, float *d_u, float *d_v,
							 float *d_w, int *d_icellflag, float *d_f, float *d_h, float *d_n, int alpha1, int alpha2,
							 float dx, float dy, float dz, float *d_dz_array, int  nx, int  ny, int nz)
{

    int icell_face = blockDim.x*blockIdx.x+threadIdx.x;
    int k = icell_face/(nx*ny);
    int j = (icell_face - k*nx*ny)/nx;
    int i = icell_face - k*nx*ny - j*nx;
    int icell_cent = i + j*(nx-1) + k*(nx-1)*(ny-1);   /// Lineralized index for cell centered values

    if((i>= 0) && (j>= 0) && (k >= 0) && (i<nx)&&(j<ny)&&(k<nz)){

        d_u[icell_face] = d_u0[icell_face];
        d_v[icell_face] = d_v0[icell_face];
        d_w[icell_face] = d_w0[icell_face];

    }


    if ((i > 0) && (i < nx-1) && (j > 0) && (j < ny-1) && (k < nz-1) && (k > 0)) {

        d_u[icell_face] = d_u0[icell_face]+(1/(2*pow(alpha1, 2.0)))*d_f[icell_cent]*dx*
						 (d_lambda[icell_cent]-d_lambda[icell_cent-1]);
        d_v[icell_face] = d_v0[icell_face]+(1/(2*pow(alpha1, 2.0)))*d_h[icell_cent]*dy*
						 (d_lambda[icell_cent]-d_lambda[icell_cent - (nx-1)]);
        d_w[icell_face] = d_w0[icell_face]+(1/(2*pow(alpha2, 2.0)))*d_n[icell_cent]*d_dz_array[k]*
						 (d_lambda[icell_cent]-d_lambda[icell_cent - (nx-1)*(ny-1)]);

    }


    if ((i >= 0) && (i < nx-1) && (j >= 0) && (j < ny-1) && (k < nz-1) && (k >= 1) && ((d_icellflag[icell_cent] == 0) || (d_icellflag[icell_cent] == 2)))
    {
        d_u[icell_face] = 0;
        d_u[icell_face+1] = 0;
        d_v[icell_face] = 0;
        d_v[icell_face+nx] = 0;
        d_w[icell_face] = 0;
        d_w[icell_face+nx*ny] = 0;

    }
}


void GlobalMemory::solve(const URBInputData* UID, URBGeneralData* UGD, bool solveWind)
{
    auto startTotal = std::chrono::high_resolution_clock::now(); // Start
                                                                 // recording
                                                                 // execution
                                                                 // time
    //int numblocks = (UGD->numcell_cent/BLOCKSIZE)+1;

    R.resize( UGD->numcell_cent, 0.0 );

    lambda.resize( UGD->numcell_cent, 0.0 );
    lambda_old.resize( UGD->numcell_cent, 0.0 );

    float *d_u0, *d_v0, *d_w0;
    float *d_u, *d_v, *d_w;
    int *d_icellflag;
    float *d_dz_array;

    auto start = std::chrono::high_resolution_clock::now(); // Start recording execution time

    cudaMalloc((void **) &d_e, UGD->numcell_cent * sizeof(float));
    cudaMalloc((void **) &d_f, UGD->numcell_cent * sizeof(float));
    cudaMalloc((void **) &d_g, UGD->numcell_cent * sizeof(float));
    cudaMalloc((void **) &d_h, UGD->numcell_cent * sizeof(float));
    cudaMalloc((void **) &d_m, UGD->numcell_cent * sizeof(float));
    cudaMalloc((void **) &d_n, UGD->numcell_cent * sizeof(float));
    cudaMalloc((void **) &d_lambda, UGD->numcell_cent * sizeof(float));
    cudaMalloc((void **) &d_u0,UGD->numcell_face*sizeof(float));
    cudaMalloc((void **) &d_v0,UGD->numcell_face*sizeof(float));
    cudaMalloc((void **) &d_w0,UGD->numcell_face*sizeof(float));
    cudaMalloc((void **) &d_dz_array,(UGD->nz-1)*sizeof(float));
    cudaMalloc((void **) &d_R, UGD->numcell_cent * sizeof(float));

    cudaMemcpy(d_u0, UGD->u0.data(),UGD->numcell_face*sizeof(float),cudaMemcpyHostToDevice);
    cudaMemcpy(d_v0, UGD->v0.data(),UGD->numcell_face*sizeof(float),cudaMemcpyHostToDevice);
    cudaMemcpy(d_w0, UGD->w0.data(),UGD->numcell_face*sizeof(float),cudaMemcpyHostToDevice);
    cudaMemcpy(d_R,R.data(),UGD->numcell_cent*sizeof(float),cudaMemcpyHostToDevice);
    cudaMemcpy(d_e , UGD->e.data() , UGD->numcell_cent * sizeof(float) , cudaMemcpyHostToDevice);
    cudaMemcpy(d_f , UGD->f.data() , UGD->numcell_cent * sizeof(float) , cudaMemcpyHostToDevice);
    cudaMemcpy(d_g , UGD->g.data() , UGD->numcell_cent * sizeof(float) , cudaMemcpyHostToDevice);
    cudaMemcpy(d_h , UGD->h.data() , UGD->numcell_cent * sizeof(float) , cudaMemcpyHostToDevice);
    cudaMemcpy(d_m , UGD->m.data() , UGD->numcell_cent * sizeof(float) , cudaMemcpyHostToDevice);
    cudaMemcpy(d_n , UGD->n.data() , UGD->numcell_cent * sizeof(float) , cudaMemcpyHostToDevice);
    cudaMemcpy(d_dz_array , UGD->dz_array.data() , (UGD->nz-1) * sizeof(float) , cudaMemcpyHostToDevice);

    dim3 numberOfThreadsPerBlock(BLOCKSIZE,1,1);
    dim3 numberOfBlocks(ceil(((UGD->nx-1)*(UGD->ny-1)*(UGD->nz-1))/(float) (BLOCKSIZE)),1,1);

    // Invoke divergence kernel
    divergenceGlobal<<<numberOfBlocks,numberOfThreadsPerBlock>>>(d_u0,d_v0,d_w0,d_R,d_e,d_f,d_g,d_h,d_m,d_n,alpha1,
                              UGD->nx, UGD->ny, UGD->nz, UGD->dx, UGD->dy, d_dz_array);

    cudaMemcpy(d_lambda , lambda.data() , UGD->numcell_cent * sizeof(float) , cudaMemcpyHostToDevice);


    /////////////////////////////////////////////////
    //                 SOR solver              //////
    /////////////////////////////////////////////////

    int iter = 0;
    double error = 1.0;

    // Main solver loop
    while ( (iter < itermax) && (error > tol))
    {
      // Save previous iteration values for error calculation
      for (int k = 0; k < UGD->nz-1; k++)
      {
        for (int j = 0; j < UGD->ny-1; j++)
        {
          for (int i = 0; i < UGD->nx-1; i++)
          {
            int ii = i + j*(UGD->nx-1) + k*(UGD->nx-1)*(UGD->ny-1);   // Lineralize the vectors (make it 1D)
            lambda_old[ii] = lambda[ii];
          }
        }
      }

      cudaMemcpy(d_lambda , lambda.data() , UGD->numcell_cent * sizeof(float) , cudaMemcpyHostToDevice);
      int offset = 0;                     // Red nodes pass
      SOR_RB_Global<<<numberOfBlocks,numberOfThreadsPerBlock>>>(d_lambda, UGD->nx, UGD->ny, UGD->nz, omega, A, B, d_e, d_f, d_g, d_h, d_m,
                                                    d_n, d_R, offset);
      cudaCheck(cudaGetLastError());

      offset = 1;                         // Black nodes pass
      SOR_RB_Global<<<numberOfBlocks,numberOfThreadsPerBlock>>>(d_lambda, UGD->nx, UGD->ny, UGD->nz, omega, A, B, d_e, d_f, d_g, d_h, d_m,
                                                    d_n, d_R, offset);
      cudaCheck(cudaGetLastError());
      cudaMemcpy (lambda.data() , d_lambda , UGD->numcell_cent * sizeof(float) , cudaMemcpyDeviceToHost);
      // Neumann boundary condition (lambda (@k=0) = lambda (@k=1))
      for (int j = 0; j < UGD->ny; j++)
      {
        for (int i = 0; i < UGD->nx; i++)
        {
          int ii = i + j*(UGD->nx-1);          // Lineralize the vectors (make it 1D)
          lambda[ii] = lambda[ii + (UGD->nx-1)*(UGD->ny-1)];
        }
      }

      error = 0.0;                     // Reset error value before error calculation

      // Error calculation
      for (int k = 0; k < UGD->nz-1; k++)
      {
        for (int j = 0; j < UGD->ny-1; j++)
        {
          for (int i = 0; i < UGD->nx-1; i++)
          {
            int ii = i + j*(UGD->nx-1) + k*(UGD->nx-1)*(UGD->ny-1);       // Lineralize the vectors (make it 1D)
            error += fabs(lambda[ii] - lambda_old[ii])/((UGD->nx-1)*(UGD->ny-1)*(UGD->nz-1));
          }
        }
      }

      iter += 1;
    }


    cudaMalloc((void **) &d_icellflag, UGD->numcell_cent * sizeof(int));
    cudaMalloc((void **) &d_u,UGD->numcell_face*sizeof(float));
    cudaMalloc((void **) &d_v,UGD->numcell_face*sizeof(float));
    cudaMalloc((void **) &d_w,UGD->numcell_face*sizeof(float));
    cudaMemcpy(d_icellflag, UGD->icellflag.data(), UGD->numcell_cent*sizeof(int),cudaMemcpyHostToDevice);

    std::cout << "Error:" << error << "\n";   // Print the number of iterations
    std::cout << "Number of iterations:" << iter << "\n";   // Print the number of iterations

    dim3 numberOfBlocks3(ceil((UGD->nx*UGD->ny*UGD->nz)/(float) (BLOCKSIZE)),1,1);
    // Invoke final velocity (Euler) kernel
    finalVelocityGlobal<<<numberOfBlocks3,numberOfThreadsPerBlock>>>(d_u0,d_v0,d_w0,d_lambda,d_u,d_v,d_w,d_icellflag,d_f,d_h,d_n,
																alpha1,alpha2,UGD->dx,UGD->dy,UGD->dz, d_dz_array,UGD->nx,UGD->ny,UGD->nz);
    cudaCheck(cudaGetLastError());

    cudaMemcpy(UGD->u.data(),d_u,UGD->numcell_face*sizeof(float),cudaMemcpyDeviceToHost);
    cudaMemcpy(UGD->v.data(),d_v,UGD->numcell_face*sizeof(float),cudaMemcpyDeviceToHost);
    cudaMemcpy(UGD->w.data(),d_w,UGD->numcell_face*sizeof(float),cudaMemcpyDeviceToHost);





    cudaFree (d_lambda);
    cudaFree (d_e);
    cudaFree (d_f);
    cudaFree (d_g);
    cudaFree (d_h);
    cudaFree (d_m);
    cudaFree (d_n);
    cudaFree (d_R);
    cudaFree (d_u0);
    cudaFree (d_v0);
    cudaFree (d_w0);
    cudaFree (d_u);
    cudaFree (d_v);
    cudaFree (d_w);
    cudaFree (d_dz_array);
    cudaFree (d_icellflag);

    auto finish = std::chrono::high_resolution_clock::now();  // Finish recording execution time

    std::chrono::duration<float> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";   // Print out elapsed execution time



}
