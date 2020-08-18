#pragma once

/*
 * This is an abstract class that is the basis for the windfield
 * convergence algorithm. This class has information needed to run
 * the simulation as well as functions widely used by different solver
 * methods
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <vector>
#include <chrono>
#include <limits>



#include "WINDSInputData.h"
#include "WINDSGeneralData.h"

#include "Vector3.h"

using namespace std;

/**< \class Solver
* This class declares and defines variables required for both solvers
*
* There are several special member variables that should be accessible
* to all solvers.  They are declared in this class.
*/
class Solver
{
protected:

    const int alpha1;        /**< Gaussian precision moduli */
    const int alpha2;        /**< Gaussian precision moduli */
    const float eta;      
    const float A;
    const float B;

    float tol;     /**< Error tolerance */
    const float omega = 1.78f;   /**< Over-relaxation factor */

    // SOLVER-based parameters
    std::vector<float> R;           /**< Divergence of initial velocity field */
    std::vector<float> lambda, lambda_old;

    int itermax;		/**< Maximum number of iterations */

    /*
     * This prints out the current amount that a process
     * has finished with a progress bar
     *
     * @param percentage -the amount the task has finished
     */
    void printProgress (float percentage);


public:
    Solver(const WINDSInputData* WID, WINDSGeneralData* WGD);

    virtual void solve(const WINDSInputData *WID, WINDSGeneralData* WGD, bool solveWind) = 0;

};
