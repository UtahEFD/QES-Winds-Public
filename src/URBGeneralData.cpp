#include "URBGeneralData.h"

URBGeneralData::URBGeneralData(const URBInputData* UID)
{
    // Determines how wakes behind buildings are calculated
    if ( UID->simParams->wakeFlag > 1)
    {
        cavity_factor = 1.1;
        wake_factor = 0.1;
    }
    else
    {
        cavity_factor = 1.0;
        wake_factor = 0.0;
    }

    // converting the domain rotation to radians from degrees -- input
    // assumes degrees
    theta = (UID->simParams->domainRotation * pi / 180.0);

    // Pull Domain Size information from the UrbInputData structure --
    // this is either read in from the XML files and/or potentially
    // calculated based on the geographic data that was loaded
    // (i.e. DEM files). It is important to get this data from the
    // main input structure.
    //
    // This is done to make reference to nx, ny and nz easier in this function
    Vector3<int> domainInfo;
    domainInfo = *(UID->simParams->domain);
    nx = domainInfo[0];
    ny = domainInfo[1];
    nz = domainInfo[2];

    // Modify the domain size to fit the Staggered Grid used in the solver
    nx += 1;        /// +1 for Staggered grid
    ny += 1;        /// +1 for Staggered grid
    nz += 2;        /// +2 for staggered grid and ghost cell

    Vector3<float> gridInfo;
    gridInfo = *(UID->simParams->grid);
    dx = gridInfo[0];		/**< Grid resolution in x-direction */
    dy = gridInfo[1];		/**< Grid resolution in y-direction */
    dz = gridInfo[2];		/**< Grid resolution in z-direction */
    dxy = MIN_S(dx, dy);

    numcell_cout    = (nx-1)*(ny-1)*(nz-2);        /**< Total number of cell-centered values in domain */
    numcell_cout_2d = (nx-1)*(ny-1);               /**< Total number of horizontal cell-centered values in domain */
    numcell_cent    = (nx-1)*(ny-1)*(nz-1);        /**< Total number of cell-centered values in domain */
    numcell_face    = nx*ny*nz;                    /**< Total number of face-centered values in domain */

    // /////////////////////////
    // Calculation of z0 domain info MAY need to move to UrbInputData
    // or somewhere else once we know the domain size
    // /////////////////////////
    z0_domain.resize( nx*ny );
    if (UID->metParams->z0_domain_flag == 0)      // Uniform z0 for the whole domain
    {
        for (auto i=0; i<nx; i++)
        {
            for (auto j=0; j<ny; j++)
            {
                id = i+j*nx;
                z0_domain[id] = UID->metParams->sensors[0]->site_z0;
            }
        }
    }
    else
    {
        for (auto i=0; i<nx/2; i++)
        {
            for (auto j=0; j<ny; j++)
            {
                id = i+j*nx;
                z0_domain[id] = 0.5;
            }
        }
        for (auto i=nx/2; i<nx; i++)
        {
            for (auto j=0; j<ny; j++)
            {
                id = i+j*nx;
                z0_domain[id] = 0.1;
            }
        }
    }

    z0 = 0.1f;
    if (UID->buildings)
        z0 = UID->buildings->wallRoughness;

    dz_array.resize( nz-1, 0.0 );
    z.resize( nz-1 );

    if (UID->simParams->verticalStretching == 0)    // Uniform vertical grid
    {
        for (auto k=1; k<z.size(); k++)
        {
            dz_array[k] = dz;
        }
    }
    else if (UID->simParams->verticalStretching == 1)     // Stretched vertical grid
    {
        for (auto k=1; k<z.size(); k++)
        {
            dz_array[k] = UID->simParams->dz_value[k-1];      // Read in custom dz values and set them to dz_array
        }
    }

    dz_array[0] = dz_array[1];                  // Value for ghost cell below the surface
    dz = *std::min_element(dz_array.begin() , dz_array.end());     // Set dz to minimum value of

    z[0] = -0.5*dz_array[0];
    for (auto k=1; k<z.size(); k++)
    {
        z[k] = z[k-1] + dz_array[k];     /**< Location of face centers in z-dir */
    }

    z_out.resize( nz-2 );
    for (size_t k=1; k<z.size(); k++)
    {
        z_out[k-1] = (float)z[k];    /**< Location of face centers in z-dir */
    }

    x.resize( nx-1 );
    x_out.resize( nx-1 );
    for (size_t i=0; i<x.size(); i++)
    {
        x_out[i] = (i+0.5)*dx;          /**< Location of face centers in x-dir */
        x[i] = (float)x_out[i];
    }

    y.resize( ny-1 );
    y_out.resize( ny-1 );
    for (auto j=0; j<ny-1; j++)
    {
        y_out[j] = (j+0.5)*dy;          /**< Location of face centers in y-dir */
        y[j] = (float)y_out[j];
    }

    // Resize the canopy-related vectors
    canopy_atten.resize( numcell_cent, 0.0 );
    canopy_top.resize( (nx-1)*(ny-1), 0.0 );
    canopy_top_index.resize( (nx-1)*(ny-1), 0 );
    canopy_z0.resize( (nx-1)*(ny-1), 0.0 );
    canopy_ustar.resize( (nx-1)*(ny-1), 0.0 );
    canopy_d.resize( (nx-1)*(ny-1), 0.0 );


    // Resize the coefficients for use with the solver e.resize( numcell_cent, 1.0 );
    e.resize( numcell_cent, 1.0 );
    f.resize( numcell_cent, 1.0 );
    g.resize( numcell_cent, 1.0 );
    h.resize( numcell_cent, 1.0 );
    m.resize( numcell_cent, 1.0 );
    n.resize( numcell_cent, 1.0 );

    icellflag.resize( numcell_cent, 1 );

    // /////////////////////////////////////////
    // Output related data --- should be part of some URBOutputData
    // class to separate from Input and GeneralData
    u_out.resize( numcell_cout, 0.0 );
    v_out.resize( numcell_cout, 0.0 );
    w_out.resize( numcell_cout, 0.0 );

    terrain.resize( numcell_cout_2d, 0.0 );
    terrain_id.resize( nx*ny, 1 );
    icellflag_out.resize( numcell_cout, 0.0 );
    /////////////////////////////////////////

    // Set the Wind Velocity data elements to be of the correct size
    // Initialize u0,v0,w0,u,v and w to 0.0
    u0.resize( numcell_face, 0.0 );
    v0.resize( numcell_face, 0.0 );
    w0.resize( numcell_face, 0.0 );


    //////////////////////////////////////////////////////////////////////////////////
    /////    Create sensor velocity profiles and generate initial velocity field /////
    //////////////////////////////////////////////////////////////////////////////////
    // Calling inputWindProfile function to generate initial velocity
    // field from sensors information (located in Sensor.cpp)

    // Pete could move to input param processing...
    assert( UID->metParams->sensors.size() > 0 );  // extra
    // check
    // to
    // be safe
    // Guaranteed to always have at least 1 sensor!
    // Pete thinks inputWindProfile should be a function of MetParams
    // so it would have access to all the sensors naturally.
    // Make this change later.
    //    UID->metParams->inputWindProfile(UID, this);
    UID->metParams->sensors[0]->inputWindProfile(UID, this);

    max_velmag = 0.0;
    for (auto i=0; i<nx; i++)
    {
        for (auto j=0; j<ny; j++)
        {
            int icell_face = i+j*nx+nz*nx*ny;
            max_velmag = MAX_S(max_velmag, sqrt(pow(u0[icell_face],2.0)+pow(v0[icell_face],2.0)));
        }
    }
    max_velmag *= 1.2;


    ////////////////////////////////////////////////////////
    //////              Apply Terrain code             /////
    ///////////////////////////////////////////////////////
    // Handle remaining Terrain processing components here
    ////////////////////////////////////////////////////////
    //
    // Behnam also notes that this section will be completely changed
    // to NOT treat terrain cells as "buildings" -- Behnam will fix
    // this
    //
    if (UID->simParams->DTE_heightField)
    {
        // ////////////////////////////////
        // Retrieve terrain height field //
        // ////////////////////////////////
        for (int i = 0; i < nx-1; i++)
        {
            for (int j = 0; j < ny-1; j++)
            {
                // Gets height of the terrain for each cell
                int idx = i + j*(nx-1);
                terrain[idx] = UID->simParams->DTE_mesh->getHeight(i * dx + dx * 0.5f, j * dy + dy * 0.5f);
                if (terrain[idx] < 0.0)
                {
                    terrain[idx] = 0.0;
                }
                id = i+j*nx;
                for (auto k=0; k<z.size(); k++)
                {
                    terrain_id[id] = k+1;
                    if (terrain[idx] < z[k+1])
                    {
                        break;
                    }
                    if (UID->simParams->meshTypeFlag == 0)
                    {
                        // ////////////////////////////////
                        // Stair-step (original QUIC)    //
                        // ////////////////////////////////
                        int icell_cent = i+j*(nx-1)+(k+1)*(nx-1)*(ny-1);
                        icellflag[icell_cent] = 2;
                    }
                }
            }
        }

        if (UID->simParams->meshTypeFlag == 1)
        {
            // ////////////////////////////////
            //        Cut-cell method        //
            // ////////////////////////////////

            // Calling calculateCoefficient function to calculate area fraction coefficients for cut-cells
            cut_cell.calculateCoefficient(cells, UID->simParams->DTE_heightField, nx, ny, nz, dx, dy, dz, n, m, f, e, h, g, pi, icellflag);
        }
    }
    ///////////////////////////////////////////////////////
    //////   END END END of  Apply Terrain code       /////
    ///////////////////////////////////////////////////////


    // Urb Input Data will have read in the specific types of
    // buildings, canopies, etc... but we need to merge all of that
    // onto a single vector of Building* -- this vector is called
    //
    // allBuildingsVector
    allBuildingsV.clear();  // make sure there's nothing on it


    // After Terrain is process, handle remaining processing of SHP
    // file data

    if (UID->simParams->SHPData)
    {
        auto buildingsetup = std::chrono::high_resolution_clock::now(); // Start recording execution time

        std::vector<Building*> poly_buildings;
        
        std::vector <float> effective_height;            // Effective height of buildings
        float corner_height, min_height;

        std::vector<float> shpDomainSize(2), minExtent(2);
        UID->simParams->SHPData->getLocalDomain( shpDomainSize );
        UID->simParams->SHPData->getMinExtent( minExtent );

        float domainOffset[2] = { 0, 0 };
        for (auto pIdx = 0; pIdx<UID->simParams->shpPolygons.size(); pIdx++)
        {
            // convert the global polys to local domain coordinates
            for (auto lIdx=0; lIdx<UID->simParams->shpPolygons[pIdx].size(); lIdx++)
            {
                UID->simParams->shpPolygons[pIdx][lIdx].x_poly -= minExtent[0] ;
                UID->simParams->shpPolygons[pIdx][lIdx].y_poly -= minExtent[1] ;
            }
        }

        // Setting base height for buildings if there is a DEM file
        if (UID->simParams->DTE_heightField && UID->simParams->DTE_mesh)
        {
            for (auto pIdx = 0; pIdx<UID->simParams->shpPolygons.size(); pIdx++)
            {
                // Get base height of every corner of building from terrain height
                min_height = UID->simParams->DTE_mesh->getHeight(UID->simParams->shpPolygons[pIdx][0].x_poly,
                                                                 UID->simParams->shpPolygons[pIdx][0].y_poly);
                if (min_height<0)
                {
                    min_height = 0.0;
                }
                for (auto lIdx=1; lIdx<UID->simParams->shpPolygons[pIdx].size(); lIdx++)
                {
                    corner_height = UID->simParams->DTE_mesh->getHeight(UID->simParams->shpPolygons[pIdx][lIdx].x_poly,
                                                                        UID->simParams->shpPolygons[pIdx][lIdx].y_poly);
                    if (corner_height<min_height && corner_height>0.0)
                    {
                        min_height = corner_height;
                    }
                }
                base_height.push_back(min_height);
            }
        }
        else
        {
            for (auto pIdx = 0; pIdx<UID->simParams->shpPolygons.size(); pIdx++)
            {
                base_height.push_back(0.0);
            }
        }

        for (auto pIdx = 0; pIdx<UID->simParams->shpPolygons.size(); pIdx++)
        {
            effective_height.push_back (base_height[pIdx]+ UID->simParams->shpBuildingHeight[pIdx]);
            for (auto lIdx=0; lIdx<UID->simParams->shpPolygons[pIdx].size(); lIdx++)
            {
                UID->simParams->shpPolygons[pIdx][lIdx].x_poly += UID->simParams->halo_x;
                UID->simParams->shpPolygons[pIdx][lIdx].y_poly += UID->simParams->halo_y;
            }
        }

        std::cout << "Creating buildings from shapefile...\n";
        // Loop to create each of the polygon buildings read in from the shapefile
        for (auto pIdx = 0u; pIdx<UID->simParams->shpPolygons.size(); pIdx++)
        {
            // Create polygon buildings
            // Pete needs to move this into URBInputData processing BUT
            // instead of adding to the poly_buildings vector, it really
            // needs to be pushed back onto buildings within the
            // UID->buildings structures...
            allBuildingsV.push_back (new PolyBuilding (UID, this, pIdx));
        }
    }


    // SHP processing is done.  Now, consolidate all "buildings" onto
    // the same list...  this includes any canopies and building types
    // that were read in via the XML file...

    // Add all the Canopy* to it (they are derived from Building)
    for (int i = 0; i < UID->canopies->canopies.size(); i++)
    {
        allBuildingsV.push_back( UID->canopies->canopies[i] );
    }

    // Add all the Building* that were read in from XML to this list
    // too -- could be RectBuilding, PolyBuilding, whatever is derived
    // from Building in the end...
    for (int i = 0; i < UID->buildings->buildings.size(); i++)
    {
        allBuildingsV.push_back( UID->buildings->buildings[i] );
    }
    

    /// all cell flags should be specific to the TYPE ofbuilding
    // class: canopy, rectbuilding, polybuilding, etc...
    // should setcellflags be part of the .. should be part of URBGeneralD
    //
    // This needs to be changed!  PolyBuildings should have been
    // created and added to buildings array if done correctly by now
    /*for (auto pIdx = 0; pIdx<shpPolygons.size(); pIdx++)
      {
      // Call setCellsFlag in the PolyBuilding class to identify building cells
      poly_buildings[pIdx].setCellsFlag ( dx, dy, dz, z, nx, ny, nz, icellflag, UID->simParams->meshTypeFlag, shpPolygons[pIdx], base_height[pIdx], building_height[pIdx]);
      }


      // Urb Input Data will have read in the specific types of
      // buildings, canopies, etc... but we need to merge all of that
      // onto a single vector of Building* -- this vector is called
      //
      // allBuildingsVector
      allBuildingsV.clear();  // make sure there's nothing on it

      // Add all the Canopy* to it (they are derived from Building)
      for (int i = 0; i < UID->canopies->canopies.size(); i++)
      {
      allBuildingsV.push_back( UID->canopies->canopies[i] );
      }

      // Add all the Building* that were read in from XML to this list
      // too -- could be RectBuilding, PolyBuilding, whatever is derived
      // from Building in the end...
      for (int i = 0; i < UID->buildings->buildings.size(); i++)
      {
      allBuildingsV.push_back( UID->buildings->buildings[i] );
      }

      // All iCellFlags should now be set!!!
      // !!!!!! Pete ---- Make sure polybuildings from SHP file get on
      // !!!!!! this list too!!!!

      // At this point, the allBuildingsV will be complete and ready for
      // use below... parameterizations, etc...

      */

    // ///////////////////////////////////////
    // ///////////////////////////////////////

    /// defining ground solid cells (ghost cells below the surface)
    for (int j = 0; j < ny-1; j++)
    {
        for (int i = 0; i < nx-1; i++)
        {
            int icell_cent = i + j*(nx-1);
            icellflag[icell_cent] = 2;
        }
    }

    //
    // Terrain stuff should go here too... somehow
    //

    // Now all buildings
    /*for (int i = 0; i < allBuildingsV.size(); i++)
      {
      // for now this does the canopy stuff for us
      allBuildingsV[i]->setCellFlags();
      }

      std::cout << "All building types (canopies, buildings, terrain) created and initialized...\n";


      // We want to sort ALL buildings here...  use the allBuildingsV to
      // do this... (remember some are canopies) so we may need a
      // virtual function in the Building class to get the appropriate
      // data for the sort.
      mergeSort( effective_height, shpPolygons, base_height, building_height);


      // ///////////////////////////////////////
      // Generic Parameterization Related Stuff
      // ///////////////////////////////////////
      for (int i = 0; i < allBuildingsV.size(); i++)
      {
      // for now this does the canopy stuff for us
      allBuildingsV[i]->callParameterizationSpecial();
      }

      // Deal with the rest of the parameterization somehow all
      // here... very generically.
      for (int i = 0; i < allBuildingsV.size(); i++)
      {
      // for now this does the canopy stuff for us
      allBuildingsV[i]->callParameterizationOne();
      }*/

#if 0

    std::cout << "Defining Solid Walls...\n";
    /// Boundary condition for building edges
    Wall->defineWall(this);
    std::cout << "Walls Defined...\n";

    /*
     * Calling wallLogBC to read in vectores of indices of the cells that have wall to right/left,
     * wall above/below and wall in front/back and applies the log law boundary condition fix
     * to the cells near Walls
     *
     */
    Wall->wallLogBC (this);

    Wall->setVelocityZero (this);

    Wall->solverCoefficients (this);
#endif
}


void URBGeneralData::mergeSort( std::vector<float> &height, std::vector<std::vector<polyVert>> &poly_points, std::vector<float> &base_height, std::vector<float> &building_height)
{
    //if the size of the array is 1, it is already sorted
    if (height.size() == 1)
    {
        return;
    }
    //make left and right sides of the data
    std::vector<float> height_L, height_R;
    std::vector<float> base_height_L, base_height_R;
    std::vector<float> building_height_L, building_height_R;
    std::vector< std::vector <polyVert> > poly_points_L, poly_points_R;
    height_L.resize(height.size() / 2);
    height_R.resize(height.size() - height.size() / 2);
    base_height_L.resize(base_height.size() / 2);
    base_height_R.resize(base_height.size() - base_height.size() / 2);
    building_height_L.resize(building_height.size() / 2);
    building_height_R.resize(building_height.size() - building_height.size() / 2);
    poly_points_L.resize(poly_points.size() / 2);
    poly_points_R.resize(poly_points.size() - poly_points.size() / 2);

    //copy data from the main data set to the left and right children
    int lC = 0, rC = 0;
    for (unsigned int i = 0; i < height.size(); i++)
    {
        if (i < height.size() / 2)
        {
            height_L[lC] = height[i];
            base_height_L[lC] = base_height[i];
            building_height_L[lC] = building_height[i];
            poly_points_L[lC++] = poly_points[i];
        }
        else
        {
            height_R[rC] = height[i];
            base_height_R[rC] = base_height[i];
            building_height_R[rC] = building_height[i];
            poly_points_R[rC++] = poly_points[i];
        }
    }

    //recursively sort the children
    mergeSort(height_L, poly_points_L, base_height_L, building_height_L);
    mergeSort(height_R, poly_points_R, base_height_R, building_height_R);

    //compare the sorted children to place the data into the main array
    lC = rC = 0;
    for (unsigned int i = 0; i < poly_points.size(); i++)
    {
        if (rC == height_R.size() || ( lC != height_L.size() &&
                                       height_L[lC] > height_R[rC]))
        {
            height[i] = height_L[lC];
            base_height[i] = base_height_L[lC];
            building_height[i] = building_height_L[lC];
            poly_points[i] = poly_points_L[lC++];
        }
        else
        {
            height[i] = height_R[rC];
            base_height[i] = base_height_R[rC];
            building_height[i] = building_height_R[rC];
            poly_points[i] = poly_points_R[rC++];
        }
    }

    return;
}




float URBGeneralData::canopyBisection(float ustar, float z0, float canopy_top, float canopy_atten, float vk, float psi_m)
{
    int iter;
    float tol, uhc, d, d1, d2, fi, fnew;

    tol = z0/100;
    fnew = tol*10;

    d1 = z0;
    d2 = canopy_top;
    d = (d1+d2)/2;

    uhc = (ustar/vk)*(log((canopy_top-d1)/z0)+psi_m);
    fi = ((canopy_atten*uhc*vk)/ustar)-canopy_top/(canopy_top-d1);

    if (canopy_atten > 0)
    {
        iter = 0;
        while (iter < 200 && abs(fnew) > tol && d < canopy_top && d > z0)
        {
            iter += 1;
            d = (d1+d2)/2;
            uhc = (ustar/vk)*(log((canopy_top-d)/z0)+psi_m);
            fnew = ((canopy_atten*uhc*vk)/ustar) - canopy_top/(canopy_top-d);
            if(fnew*fi>0)
            {
                d1 = d;
            }
            else if(fnew*fi<0)
            {
                d2 = d;
            }
        }
        if (d > canopy_top)
        {
            d = 10000;
        }
    }
    else
    {
        d = 0.99*canopy_top;
    }

    return d;
}



URBGeneralData::URBGeneralData()
{
}

URBGeneralData::~URBGeneralData()
{
}
