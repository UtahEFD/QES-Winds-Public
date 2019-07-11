#pragma once

#define _USE_MATH_DEFINES
#define MIN_S(x,y) ((x) < (y) ? (x) : (y))
#define MAX_S(x,y) ((x) > (y) ? (x) : (y))

class URBGeneralData {
protected:
    URBGeneralData();
    
public:
    URBGeneralData(const URBInputData *UID);
    ~URBGeneralData();
    
    

    ////////////////////////////////////////////////////////////////////////////
    //////// Variables and constants needed only in other functions-- Behnam
    //////// This can be moved to a new class (URBGeneralData)
    ////////////////////////////////////////////////////////////////////////////
    const float pi = 4.0f * atan(1.0);
    const float vk = 0.4;			/// Von Karman's
                                                /// constant
    float cavity_factor, wake_factor;
    float theta;
    
    // General QUIC Domain Data
    int nx, ny, nz;		/**< number of cells */
    float dx, dy, dz;		/**< Grid resolution*/
    float dxy;		/**< Minimum value between dx and dy */

    long numcell_cout;
    long numcell_cout_2d;
    long numcell_cent;       /**< Total number of cell-centered values in domain */
    long numcell_face;       /**< Total number of face-centered values in domain */
    int icell_face;          /**< cell-face index */
    int icell_cent;

    std::vector<float> z0_domain;

    std::vector<Building*> allBuildingsV;

    float z0;           // In wallLogBC

    std::vector<float> dz_array;
    std::vector<float> x,y,z;    
    std::vector<double> x_out,y_out,z_out;


    

    int id;

    // Just for output

    std::vector<double> u_out,v_out,w_out;
    std::vector<int> icellflag_out;
    std::vector<double> terrain;
    // Sensor function (inputWindProfile)
    std::vector<int> terrain_id;
    Sensor* sensor;

    std::vector<float> site_canopy_H;
    std::vector<float> site_atten_coeff;


    float convergence;
    // Canopy functions

    std::vector<float> atten;		/**< Attenuation coefficient */

    int lu_canopy_flag;

    Canopy* canopy;

    float max_velmag;         // In polygonWake

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

};

