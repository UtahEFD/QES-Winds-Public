#include "CPUSolver.h"

using std::cerr;
using std::endl;
using std::vector;
using std::cout;

void CPUSolver::solve(const URBInputData* UID, URBGeneralData* UGD, bool solveWind)
{
    auto startOfSolveMethod = std::chrono::high_resolution_clock::now(); // Start recording execution time

    /////////////////////////////////////////////////////////////////
    ////////      Divergence of the initial velocity field   ////////
    /////////////////////////////////////////////////////////////////

    int icell_face;          /**< cell-face index */
    int icell_cent;

    for (int k = 1; k < UGD->nz-1; k++)
    {
        for (int j = 0; j < UGD->ny-1; j++)
        {
            for (int i = 0; i < UGD->nx-1; i++)
            {
                icell_cent = i + j*(UGD->nx-1) + k*(UGD->nx-1)*(UGD->ny-1);
                icell_face = i + j*UGD->nx + k*UGD->nx*UGD->ny;

                /// Calculate divergence of initial velocity field
                R[icell_cent] = (-2*pow(alpha1, 2.0))*((( UGD->e[icell_cent] * UGD->u0[icell_face+1]       - UGD->f[icell_cent] * UGD->u0[icell_face]) * UGD->dx ) +
                                                       (( UGD->g[icell_cent] * UGD->v0[icell_face + UGD->nx]    - UGD->h[icell_cent] * UGD->v0[icell_face]) * UGD->dy ) +
                                                       ( UGD->m[icell_cent]  * UGD->w0[icell_face + UGD->nx*UGD->ny] * UGD->dz_array[k]*0.5*(UGD->dz_array[k]+UGD->dz_array[k+1])
                                                        - UGD->n[icell_cent] * UGD->w0[icell_face] * UGD->dz_array[k]*0.5*(UGD->dz_array[k]+UGD->dz_array[k-1]) ));
            }
        }
    }


    if (solveWind)
    {
        auto startSolveSection = std::chrono::high_resolution_clock::now();

        //INSERT CANOPY CODE

        /////////////////////////////////////////////////
        //                 SOR solver              //////
        /////////////////////////////////////////////////
        int iter = 0;
        double error = 1.0;
    	  double reduced_error = 0.0;

        std::cout << "Solving...\n";
        while (iter < itermax && error > tol && error > reduced_error) {

            // Save previous iteration values for error calculation
            //    uses stl vector's assignment copy function, assign
            lambda_old.assign( lambda.begin(), lambda.end() );

            //
            // main SOR formulation loop
            //
            for (int k = 1; k < UGD->nz-2; k++){
            	for (int j = 1; j < UGD->ny-2; j++){
            	    for (int i = 1; i < UGD->nx-2; i++){

            	        icell_cent = i + j*(UGD->nx-1) + k*(UGD->nx-1)*(UGD->ny-1);   /// Lineralized index for cell centered values

                        lambda[icell_cent] = (omega / ( UGD->e[icell_cent] + UGD->f[icell_cent] + UGD->g[icell_cent] +
                                                        UGD->h[icell_cent] + UGD->m[icell_cent] + UGD->n[icell_cent])) *
                            ( UGD->e[icell_cent] * lambda[icell_cent+1]        + UGD->f[icell_cent] * lambda[icell_cent-1] +
                              UGD->g[icell_cent] * lambda[icell_cent + (UGD->nx-1)] + UGD->h[icell_cent] * lambda[icell_cent-(UGD->nx-1)] +
                              UGD->m[icell_cent] * lambda[icell_cent+(UGD->nx-1)*(UGD->ny-1)] +
                              UGD->n[icell_cent] * lambda[icell_cent-(UGD->nx-1)*(UGD->ny-1)] - R[icell_cent] ) +
                            (1.0 - omega) * lambda[icell_cent];    /// SOR formulation
                    }
                }
            }

            /// Mirror boundary condition (lambda (@k=0) = lambda (@k=1))
            for (int j = 0; j < UGD->ny-1; j++){
                for (int i = 0; i < UGD->nx-1; i++){
                    int icell_cent = i + j*(UGD->nx-1);         /// Lineralized index for cell centered values
                    lambda[icell_cent] = lambda[icell_cent + (UGD->nx-1)*(UGD->ny-1)];
                }
            }

            /// Error calculation
            error = 0.0;                   /// Reset error value before error calculation
            for (int k = 0; k < UGD->nz-1; k++){
                for (int j = 0; j < UGD->ny-1; j++){
                    for (int i = 0; i < UGD->nx-1; i++){
                        int icell_cent = i + j*(UGD->nx-1) + k*(UGD->nx-1)*(UGD->ny-1);   /// Lineralized index for cell centered values
                        error += fabs(lambda[icell_cent] - lambda_old[icell_cent])/((UGD->nx-1)*(UGD->ny-1)*(UGD->nz-1));
                    }
                }
            }

            if (iter == 0) {
                reduced_error = error * 1.0e-3;
            }

            iter += 1;
        }
        std::cout << "Solved!\n";

        std::cout << "Number of iterations:" << iter << "\n";   // Print the number of iterations
        std::cout << "Error:" << error << "\n";
        std::cout << "Reduced Error:" << reduced_error << "\n";

        ////////////////////////////////////////////////////////////////////////
        /////   Update the velocity field using Euler-Lagrange equations   /////
        ////////////////////////////////////////////////////////////////////////

        for (int k = 0; k < UGD->nz; k++)
        {
            for (int j = 0; j < UGD->ny; j++)
            {
                for (int i = 0; i < UGD->nx; i++)
                {
                    int icell_face = i + j*UGD->nx + k*UGD->nx*UGD->ny;   /// Lineralized index for cell faced values
                    UGD->u[icell_face] = UGD->u0[icell_face];
                    UGD->v[icell_face] = UGD->v0[icell_face];
                    UGD->w[icell_face] = UGD->w0[icell_face];
                }
            }
        }


        // /////////////////////////////////////////////
    	  /// Update velocity field using Euler equations
        // /////////////////////////////////////////////
        for (int k = 1; k < UGD->nz-1; k++)
        {
            for (int j = 1; j < UGD->ny-1; j++)
            {
                for (int i = 1; i < UGD->nx-1; i++)
                {
                    icell_cent = i + j*(UGD->nx-1) + k*(UGD->nx-1)*(UGD->ny-1);   /// Lineralized index for cell centered values
                    icell_face = i + j*UGD->nx + k*UGD->nx*UGD->ny;               /// Lineralized index for cell faced values

                    UGD->u[icell_face] = UGD->u0[icell_face] + (1/(2*pow(alpha1, 2.0))) *
                        UGD->f[icell_cent]*UGD->dx*(lambda[icell_cent]-lambda[icell_cent-1]);

                        // Calculate correct wind velocity
                    UGD->v[icell_face] = UGD->v0[icell_face] + (1/(2*pow(alpha1, 2.0))) *
                        UGD->h[icell_cent]*UGD->dy*(lambda[icell_cent]-lambda[icell_cent - (UGD->nx-1)]);

                    UGD->w[icell_face] = UGD->w0[icell_face]+(1/(2*pow(alpha2, 2.0))) *
                        UGD->n[icell_cent]*UGD->dz_array[k]*(lambda[icell_cent]-lambda[icell_cent - (UGD->nx-1)*(UGD->ny-1)]);
                }
            }
        }

        for (int k = 1; k < UGD->nz-1; k++)
        {
            for (int j = 0; j < UGD->ny-1; j++)
            {
                for (int i = 0; i < UGD->nx-1; i++)
                {
                    icell_cent = i + j*(UGD->nx-1) + k*(UGD->nx-1)*(UGD->ny-1);   /// Lineralized index for cell centered values
                    icell_face = i + j*UGD->nx + k*UGD->nx*UGD->ny;               /// Lineralized index for cell faced values

                    // If we are inside a building, set velocities to 0.0
                    if (UGD->icellflag[icell_cent] == 0 || UGD->icellflag[icell_cent] == 2)
                    {
                        /// Setting velocity field inside the building to zero
                        UGD->u[icell_face] = 0;
                        UGD->u[icell_face+1] = 0;
                        UGD->v[icell_face] = 0;
                        UGD->v[icell_face+UGD->nx] = 0;
                        UGD->w[icell_face] = 0;
                        UGD->w[icell_face+UGD->nx*UGD->ny] = 0;
                    }
                }
            }
        }

        auto finish = std::chrono::high_resolution_clock::now();  // Finish recording execution time
        std::chrono::duration<float> elapsedTotal = finish - startOfSolveMethod;
        std::chrono::duration<float> elapsedSolve = finish - startSolveSection;
        std::cout << "Elapsed total time: " << elapsedTotal.count() << " s\n";   // Print out elapsed execution time
        std::cout << "Elapsed solve time: " << elapsedSolve.count() << " s\n";   // Print out elapsed execution time

    }


}
