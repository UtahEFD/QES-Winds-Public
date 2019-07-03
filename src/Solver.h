#pragma once

/*
 * This is an abstract class that is the basis for the windfield
 * convergence algorithm. This class has information needed to run
 * the simulation as well as functions widely used by different solver
 * methods
 */

#include "URBInputData.h"
#include "SimulationParameters.h"
#include "Buildings.h"
#include "NonPolyBuilding.h"
#include "RectangularBuilding.h"
#include "Vector3.h"
#include "Output.hpp"
#include "Mesh.h"
#include "DTEHeightField.h"
#include "Cell.h"
#include "Sensor.h"
#include "Canopies.h"
#include "Canopy.h"
#include "Cut_cell.h"
#include "PolyBuilding.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <vector>
#include <chrono>
#include <limits>

using namespace std;

#define _USE_MATH_DEFINES
#define MIN_S(x,y) ((x) < (y) ? (x) : (y))
#define MAX_S(x,y) ((x) > (y) ? (x) : (y))

/**< \class Solver
* This class declares and defines variables required for both solvers
*
* There are several special member variables that should be accessible
* to all solvers.  They are declared in this class.
*/
class Solver
{
protected:

    ////////////////////////////////////////////////////////////////////////////
    //////// Variables and constants needed only in solver -- Behnam
    //////// These can be stayed in the solver class
    ////////////////////////////////////////////////////////////////////////////
    const float eta = pow(alpha1/alpha2, 2.0);
    const float A = pow(dx/dy, 2.0);
    const float B = eta*pow(dx/dz, 2.0);
    const float tol = 1.0e-9;     /**< Error tolerance -->>> very
                                   * small tol for float! */
    const float omega = 1.78f;   /**< Over-relaxation factor */
    // SOLVER-based parameters
    int itermax;		/**< Maximum number of iterations */
    std::vector<double> R;           /**< Divergence of initial velocity field */
    /// Declaration of final velocity field components (u,v,w)
    std::vector<double> u,v,w;
    /// Declaration of Lagrange multipliers
    std::vector<double> lambda, lambda_old;
    std::vector<int> icellflag;           /**< Cell index flag (0 = building/terrain, 1 = fluid) */

    ////////////////////////////////////////////////////////////////////////////
    //////// Variables and constants needed in solver and other functions-- Behnam
    ////////////////////////////////////////////////////////////////////////////
    // Matthieu will use change them in future
    const int alpha1 = 1;        /**< Gaussian precision moduli */
    const int alpha2 = 1;        /**< Gaussian precision moduli */
    // General QUIC Domain Data
    int nx, ny, nz;		/**< number of cells */
    float dx, dy, dz;		/**< Grid resolution*/
    float dxy;		/**< Minimum value between dx and dy */
    std::vector<float> dz_array;
    /// Declaration of coefficients for SOR solver
    std::vector<float> e,f,g,h,m,n;
    /// Declaration of initial wind components (u0,v0,w0)
    std::vector<double> u0,v0,w0;
    long numcell_cout;
    long numcell_cout_2d;
    long numcell_cent;                          /**< Total number of cell-centered values in domain */
    long numcell_face;                          /**< Total number of face-centered values in domain */
    int icell_face;                             /**< cell-face index */
    int icell_cent;

    ////////////////////////////////////////////////////////////////////////////
    //////// Variables and constants needed only in other functions-- Behnam
    //////// This can be moved to a new class (URBGeneralData)
    ////////////////////////////////////////////////////////////////////////////
    const float pi = 4.0f * atan(1.0);
    const float vk = 0.4;			/// Von Karman's constant
    int id;
    std::vector<float> x,y,z;
    // Just for output
    std::vector<double> x_out,y_out,z_out;
    std::vector<double> u_out,v_out,w_out;
    std::vector<int> icellflag_out;
    std::vector<double> terrain;
    // Sensor function (inputWindProfile)
    std::vector<int> terrain_id;
    Sensor* sensor;
    std::vector<float> z0_domain;
    std::vector<float> site_canopy_H;
    std::vector<float> site_atten_coeff;
    float UTMx, UTMy;       /**< Origin of QUIC domain UTM coordinates */
    int UTMZone;            /**< Domain UTM zone */
    float domain_rotation, theta;
    float convergence;
    // Canopy functions
    int num_canopies;				/**< number of canopies */
    std::vector<float> atten;		/**< Attenuation coefficient */
    int landuse_flag;
    int landuse_veg_flag;
    int landuse_urb_flag;
    int lu_canopy_flag;
    std::vector<Building*> canopies;
    Canopy* canopy;
    float z0;           // In wallLogBC
    float max_velmag;         // In polygonWake
    std::vector<Building*> buildings;
    // In getWallIndices and wallLogBC
    std::vector<int> wall_right_indices;     /**< Indices of the cells with wall to right boundary condition */
    std::vector<int> wall_left_indices;      /**< Indices of the cells with wall to left boundary condition */
    std::vector<int> wall_above_indices;     /**< Indices of the cells with wall above boundary condition */
    std::vector<int> wall_below_indices;     /**< Indices of the cells with wall bellow boundary condition */
    std::vector<int> wall_back_indices;      /**< Indices of the cells with wall in back boundary condition */
    std::vector<int> wall_front_indices;     /**< Indices of the cells with wall in front boundary condition */
    Mesh* mesh;           // In terrain functions
    Cell* cells;
    bool DTEHFExists = false;
    Cut_cell cut_cell;
    // Flags needed for parameterizations
    int rooftopFlag;	     	/**< Rooftop flag */
    int upwindCavityFlag;	    	/**< Upwind cavity flag */
    int streetCanyonFlag;	    	/**< Street canyon flag */
    int streetIntersectionFlag;	    	/**< Street intersection flag */
    int wakeFlag;	           	/**< Wake flag */
    int sidewallFlag;	       	/**< Sidewall flag */
    int mesh_type_flag;     	/**< mesh type (0 = Original QUIC/Stair-step, 1 = Cut-cell method) */
    float cavity_factor, wake_factor;
    // Building cut-cell (rectangular building)
    std::vector<std::vector<std::vector<float>>> x_cut;
    std::vector<std::vector<std::vector<float>>> y_cut;
    std::vector<std::vector<std::vector<float>>> z_cut;
    std::vector<std::vector<int>> num_points;
    std::vector<std::vector<float>> coeff;
    /// Declaration of output manager
    int output_counter=0;
    double time=0;
    std::vector<NcDim> dim_scalar_t;
    std::vector<NcDim> dim_scalar_z;
    std::vector<NcDim> dim_scalar_y;
    std::vector<NcDim> dim_scalar_x;
    std::vector<NcDim> dim_vector;
    std::vector<NcDim> dim_vector_2d;
    std::vector<std::string> output_fields;
    struct AttScalarDbl {
        double* data;
        std::string name;
        std::string long_name;
        std::string units;
        std::vector<NcDim> dimensions;
    };
    struct AttVectorDbl {
        std::vector<double>* data;
        std::string name;
        std::string long_name;
        std::string units;
        std::vector<NcDim> dimensions;
    };
    struct AttVectorInt {
        std::vector<int>* data;
        std::string name;
        std::string long_name;
        std::string units;
        std::vector<NcDim> dimensions;
    };
    std::map<std::string,AttScalarDbl> map_att_scalar_dbl;
    std::map<std::string,AttVectorDbl> map_att_vector_dbl;
    std::map<std::string,AttVectorInt> map_att_vector_int;
    std::vector<AttScalarDbl> output_scalar_dbl;
    std::vector<AttVectorDbl> output_vector_dbl;
    std::vector<AttVectorInt> output_vector_int;

    /*
     * This prints out the current amount that a process
     * has finished with a progress bar
     *
     * @param percentage -the amount the task has finished
     */
    void printProgress (float percentage);



public:
    Solver(const URBInputData* UID, const DTEHeightField* DTEHF, Output* output);

    virtual void solve(bool solveWind) = 0;

    /**
     * @brief
     *
     * This function takes in values necessary for cut-cell method for
     * buildings and then calculates the area fraction coefficients,
     * sets them to approperiate solver coefficients and finally sets
     * related coefficients to zero to define solid walls for non
     * cut-cells.
     */
    void calculateCoefficients(float dx, float dy, float dz, int nx, int ny, int nz, std::vector<int> &icellflag, float* n, float* m,
                     float* f, float* e, float* h, float* g, std::vector<std::vector<std::vector<float>>> x_cut,
                     std::vector<std::vector<std::vector<float>>>y_cut,std::vector<std::vector<std::vector<float>>> z_cut,
                     std::vector<std::vector<int>> num_points, std::vector<std::vector<float>> coeff);
    /**
     * @brief
     *
     * This function takes in the icellflags set by setCellsFlag
     * function for stair-step method and sets related coefficients to
     * zero to define solid walls.
     */
    void defineWalls(float dx, float dy, float dz, int nx, int ny, int nz, std::vector<int> &icellflag, float* n, float* m,
                     float* f, float* e, float* h, float* g);


    /**
    * @brief
    *
    * This function takes in the icellflag valus set by setCellsFlag
    * function for both stair-step and cut-cell methods and creates vectors of indices
    * of the cells that have wall to right/left, wall above/bellow and wall in front/back
    *
    */
    void getWallIndices (std::vector<int>& icellflag, std::vector<int>& wall_right_indices, std::vector<int>& wall_left_indices,
                        std::vector<int>& wall_above_indices, std::vector<int>& wall_below_indices,
                        std::vector<int>& wall_front_indices, std::vector<int>& wall_back_indices);

    /**
    * @brief
    *
    * This function takes in vectores of indices of the cells that have wall to right/left,
    * wall above/bellow and wall in front/back and applies the log law boundary condition fix
    * to the cells near Walls (based on Kevin Briggs master's thesis (2015))
    *
    */
    void wallLogBC (std::vector<int>& wall_right_indices, std::vector<int>& wall_left_indices,
                    std::vector<int>& wall_above_indices, std::vector<int>& wall_below_indices,
                    std::vector<int>& wall_front_indices, std::vector<int>& wall_back_indices,
                    double *u0, double *v0, double *w0, float z0);


    void mergeSort( std::vector<float> &height, std::vector<std::vector<polyVert>> &poly_points, std::vector<float> &base_height, std::vector<float> &building_height);

    /**
    * @brief
    *
    * This function saves user-defined data to file
    */
    void save(Output*);
};
