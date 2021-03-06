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


#include "WINDSGeneralData.h"

WINDSGeneralData::WINDSGeneralData(const WINDSInputData* WID, int solverType)
{
   if ( WID->simParams->upwindCavityFlag == 1) {
      lengthf_coeff = 2.0;
   } else {
      lengthf_coeff = 1.5;
   }

   // Determines how wakes behind buildings are calculated
   if ( WID->simParams->wakeFlag > 1) {
      cavity_factor = 1.1;
      wake_factor = 0.1;
   } else {
      cavity_factor = 1.0;
      wake_factor = 0.0;
   }

   // converting the domain rotation to radians from degrees -- input
   // assumes degrees
   theta = (WID->simParams->domainRotation * pi / 180.0);

   // Pull Domain Size information from the WINDSInputData structure --
   // this is either read in from the XML files and/or potentially
   // calculated based on the geographic data that was loaded
   // (i.e. DEM files). It is important to get this data from the
   // main input structure.
   //
   // This is done to make reference to nx, ny and nz easier in this function
   Vector3<int> domainInfo;
   domainInfo = *(WID->simParams->domain);
   nx = domainInfo[0];
   ny = domainInfo[1];
   nz = domainInfo[2];

   // Modify the domain size to fit the Staggered Grid used in the solver
   nx += 1;        /// +1 for Staggered grid
   ny += 1;        /// +1 for Staggered grid
   nz += 2;        /// +2 for staggered grid and ghost cell

   Vector3<float> gridInfo;
   gridInfo = *(WID->simParams->grid);
   dx = gridInfo[0];           /**< Grid resolution in x-direction */
   dy = gridInfo[1];           /**< Grid resolution in y-direction */
   dz = gridInfo[2];           /**< Grid resolution in z-direction */
   dxy = MIN_S(dx, dy);

   numcell_cout    = (nx-1)*(ny-1)*(nz-2);        /**< Total number of cell-centered values in domain */
   numcell_cout_2d = (nx-1)*(ny-1);               /**< Total number of horizontal cell-centered values in domain */
   numcell_cent    = (nx-1)*(ny-1)*(nz-1);        /**< Total number of cell-centered values in domain */
   numcell_face    = nx*ny*nz;                    /**< Total number of face-centered values in domain */


   // where should this really go?
   //
   // Need to now take all WRF station data and convert to
   // sensors
   /*if (WID->simParams->wrfInputData)
   {

      WRFInput *wrf_ptr = WID->simParams->wrfInputData;

      std::cout << "Size of stat data: " << wrf_ptr->statData.size() << std::endl;
      WID->metParams->sensors.resize( wrf_ptr->statData.size() );

      for (size_t i=0; i<wrf_ptr->statData.size(); i++) {
         std::cout << "Station " << i << " ("
                   << wrf_ptr->statData[i].xCoord << ", "
                   << wrf_ptr->statData[i].yCoord << ")" << std::endl;

         if (!WID->metParams->sensors[i])
            WID->metParams->sensors[i] = new Sensor();

         WID->metParams->sensors[i]->site_xcoord = wrf_ptr->statData[i].xCoord;
         WID->metParams->sensors[i]->site_ycoord = wrf_ptr->statData[i].yCoord;

         // WRF profile data -- sensor blayer flag is 4
         WID->metParams->sensors[i]->TS[0]->site_blayer_flag = 4;

         // Make sure to set size_z0 to be z0 from WRF cell
         WID->metParams->sensors[i]->TS[0]->site_z0 = wrf_ptr->statData[i].z0;

         //
         // 1 time series for now - how do we deal with this for
         // new time steps???  Need to figure out ASAP.
         //
         for (int t=0; t<1; t++) {
            std::cout << "\tTime Series: " << t << std::endl;

            int profDataSz = wrf_ptr->statData[i].profiles[t].size();
            WID->metParams->sensors[i]->TS[0]->site_wind_dir.resize( profDataSz );
            WID->metParams->sensors[i]->TS[0]->site_z_ref.resize( profDataSz );
            WID->metParams->sensors[i]->TS[0]->site_U_ref.resize( profDataSz );


            for (size_t p=0; p<wrf_ptr->statData[i].profiles[t].size(); p++) {
               std::cout << "\t" << wrf_ptr->statData[i].profiles[t][p].zCoord
                         << ", " << wrf_ptr->statData[i].profiles[t][p].ws
                         << ", " << wrf_ptr->statData[i].profiles[t][p].wd << std::endl;

               WID->metParams->sensors[i]->TS[0]->site_z_ref[p] = wrf_ptr->statData[i].profiles[t][p].zCoord;
               WID->metParams->sensors[i]->TS[0]->site_U_ref[p] = wrf_ptr->statData[i].profiles[t][p].ws;
               WID->metParams->sensors[i]->TS[0]->site_wind_dir[p] = wrf_ptr->statData[i].profiles[t][p].wd;

            }
         }
      }

   }*/

   // /////////////////////////
   // Calculation of z0 domain info MAY need to move to WINDSInputData
   // or somewhere else once we know the domain size
   // /////////////////////////
   z0_domain_u.resize( nx*ny );
   z0_domain_v.resize( nx*ny );
   if (WID->metParams->z0_domain_flag == 0)      // Uniform z0 for the whole domain
   {
      for (auto i=0; i<nx; i++)
      {
         for (auto j=0; j<ny; j++)
         {
            id = i+j*nx;
            z0_domain_u[id] = WID->metParams->sensors[0]->TS[0]->site_z0;
            z0_domain_v[id] = WID->metParams->sensors[0]->TS[0]->site_z0;
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
            z0_domain_u[id] = 0.5;
            z0_domain_v[id] = 0.5;
         }
      }
      for (auto i=nx/2; i<nx; i++)
      {
         for (auto j=0; j<ny; j++)
         {
            id = i+j*nx;
            z0_domain_u[id] = 0.1;
            z0_domain_v[id] = 0.1;
         }
      }
   }

   z0 = 0.1f;
   if (WID->buildings)
      z0 = WID->buildings->wallRoughness;

   // /////////////////////////
   // Definition of the grid
   // /////////////////////////

   // vertical grid (can be uniform or stretched)
   dz_array.resize( nz-1, 0.0 );
   z.resize( nz-1 );
   z_face.resize( nz-1 );

   if (WID->simParams->verticalStretching == 0) {        // Uniform vertical grid
      for (size_t k=1; k<z.size(); k++) {
         dz_array[k] = dz;
      }
   } else if (WID->simParams->verticalStretching == 1) { // Stretched vertical grid
      for (size_t k=1; k<z.size(); k++) {
         dz_array[k] = WID->simParams->dz_value[k-1];  // Read in custom dz values and set them to dz_array
      }
   }

   dz_array[0] = dz_array[1];                  // Value for ghost cell below the surface
   dz = *std::min_element(dz_array.begin() , dz_array.end());     // Set dz to minimum value of

   z_face[0] = 0.0;
   z[0] = -0.5*dz_array[0];
   for (size_t k=1; k<z.size(); k++) {
      z_face[k] = z_face[k-1] + dz_array[k];  /**< Location of face centers in z-dir */
      z[k] = 0.5*(z_face[k-1] + z_face[k]);   /**< Location of cell centers in z-dir */
   }


   // horizontal grid (x-direction)
   x.resize( nx-1 );
   for (auto i=0; i<nx-1; i++) {
      x[i] = (i+0.5)*dx;          /**< Location of face centers in x-dir */
   }

   // horizontal grid (y-direction)
   y.resize( ny-1 );
   for (auto j=0; j<ny-1; j++) {
      y[j] = (j+0.5)*dy;          /**< Location of face centers in y-dir */
   }

   // Resize the canopy-related vectors
   canopy_atten.resize( numcell_cent, 0.0 );
   canopy_top.resize( (nx-1)*(ny-1), 0.0 );
   canopy_top_index.resize( (nx-1)*(ny-1), 0 );
   canopy_z0.resize( (nx-1)*(ny-1), 0.0 );
   canopy_ustar.resize( (nx-1)*(ny-1), 0.0 );
   canopy_d.resize( (nx-1)*(ny-1), 0.0 );


   // Resize the coefficients for use with the solver
   e.resize( numcell_cent, 1.0 );
   f.resize( numcell_cent, 1.0 );
   g.resize( numcell_cent, 1.0 );
   h.resize( numcell_cent, 1.0 );
   m.resize( numcell_cent, 1.0 );
   n.resize( numcell_cent, 1.0 );

   /*building_volume_frac.resize( numcell_cent, 1.0 );
   terrain_volume_frac.resize( numcell_cent, 1.0 ); */

   icellflag.resize( numcell_cent, 1 );
   ibuilding_flag.resize ( numcell_cent, -1 );

   //mixingLengths.resize( numcell_cent, 0.0 );

   terrain.resize( numcell_cout_2d, 0.0 );
   terrain_id.resize( nx*ny, 1 );

   /////////////////////////////////////////

   // Set the Wind Velocity data elements to be of the correct size
   // Initialize u0,v0,w0,u,v and w to 0.0
   u0.resize( numcell_face, 0.0 );
   v0.resize( numcell_face, 0.0 );
   w0.resize( numcell_face, 0.0 );

   u.resize( numcell_face, 0.0 );
   v.resize( numcell_face, 0.0 );
   w.resize( numcell_face, 0.0 );

   std::cout << "Memory allocation complete." << std::endl;

   /// defining ground solid cells (ghost cells below the surface)
   for (int j = 0; j < ny-1; j++)
   {
      for (int i = 0; i < nx-1; i++)
      {
         int icell_cent = i + j*(nx-1);
         icellflag[icell_cent] = 2;
      }
   }

   int halo_index_x = (WID->simParams->halo_x/dx);
   //WID->simParams->halo_x = halo_index_x*dx;
   int halo_index_y = (WID->simParams->halo_y/dy);
   //WID->simParams->halo_y = halo_index_y*dy;

   if (WID->simParams->DTE_heightField)
   {
      // ////////////////////////////////
      // Retrieve terrain height field //
      // ////////////////////////////////
      for (int i = 0; i < nx-halo_index_x-1; i++)
      {
         for (int j = 0; j < ny-halo_index_y-1; j++)
         {
            // Gets height of the terrain for each cell
            int ii = i+WID->simParams->halo_x/dx;
            int jj = j+WID->simParams->halo_y/dy;
            int idx = ii + jj*(nx-1);
            terrain[idx] = WID->simParams->DTE_mesh->getHeight(i * dx + dx * 0.5f, j * dy + dy * 0.5f);
            if (terrain[idx] < 0.0)
            {
               terrain[idx] = 0.0;
            }
            id = ii+jj*nx;
            for (size_t k=0; k<z.size()-1; k++)
            {
               terrain_id[id] = k;
               if (terrain[idx] < z_face[k])
               {
                  break;
               }
            }
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////////////
   /////    Create sensor velocity profiles and generate initial velocity field /////
   //////////////////////////////////////////////////////////////////////////////////
   // Calling inputWindProfile function to generate initial velocity
   // field from sensors information (located in Sensor.cpp)

   // Pete could move to input param processing...
   assert( WID->metParams->sensors.size() > 0 );  // extra
   // check
   // to
   // be safe
   // Guaranteed to always have at least 1 sensor!
   // Pete thinks inputWindProfile should be a function of MetParams
   // so it would have access to all the sensors naturally.
   // Make this change later.
   //    WID->metParams->inputWindProfile(WID, this);
   WID->metParams->sensors[0]->inputWindProfile(WID, this, 0, solverType);

   std::cout << "Sensors have been loaded (total sensors = " << WID->metParams->sensors.size() << ")." << std::endl;

   ////////////////////////////////////////////////////////
   //////              Apply Terrain code             /////
   ///////////////////////////////////////////////////////
   // Handle remaining Terrain processing components here
   ////////////////////////////////////////////////////////


   if (WID->simParams->DTE_heightField)
   {

      if (WID->simParams->meshTypeFlag == 0 && WID->simParams->readCoefficientsFlag == 0)
      {
        auto start_stair = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < nx-halo_index_x-1; i++)
        {
           for (int j = 0; j < ny-halo_index_y-1; j++)
           {
              // Gets height of the terrain for each cell
              int ii = i+WID->simParams->halo_x/dx;
              int jj = j+WID->simParams->halo_y/dy;
              int idx = ii + jj*(nx-1);
              for (size_t k=0; k<z.size()-1; k++)
              {
                 if (terrain[idx] < z[k+1])
                 {
                    break;
                 }

                 // ////////////////////////////////
                 // Stair-step (original QUIC)    //
                 // ////////////////////////////////
                 int icell_cent = ii+jj*(nx-1)+(k+1)*(nx-1)*(ny-1);
                 icellflag[icell_cent] = 2;

               }
            }
        }

        auto finish_stair = std::chrono::high_resolution_clock::now();  // Finish recording execution time

        std::chrono::duration<float> elapsed_stair = finish_stair - start_stair;
        std::cout << "Elapsed time for terrain with stair-step: " << elapsed_stair.count() << " s\n";
      }
   }
   ///////////////////////////////////////////////////////
   //////   END of  Apply Terrain code       /////
   ///////////////////////////////////////////////////////


   // WINDS Input Data will have read in the specific types of
   // buildings, canopies, etc... but we need to merge all of that
   // onto a single vector of Building* -- this vector is called
   //
   // allBuildingsVector
   allBuildingsV.clear();  // make sure there's nothing on it

   // After Terrain is processed, handle remaining processing of SHP
   // file data

   if (WID->simParams->SHPData)
   {
      auto buildingsetup_start = std::chrono::high_resolution_clock::now(); // Start recording execution time

      std::vector<Building*> poly_buildings;


      float corner_height, min_height;

      std::vector<float> shpDomainSize(2), minExtent(2);
      WID->simParams->SHPData->getLocalDomain( shpDomainSize );
      WID->simParams->SHPData->getMinExtent( minExtent );

      // float domainOffset[2] = { 0, 0 };
      for (auto pIdx = 0u; pIdx<WID->simParams->shpPolygons.size(); pIdx++)
      {
         // convert the global polys to local domain coordinates
         for (auto lIdx=0u; lIdx<WID->simParams->shpPolygons[pIdx].size(); lIdx++)
         {
            WID->simParams->shpPolygons[pIdx][lIdx].x_poly -= minExtent[0] ;
            WID->simParams->shpPolygons[pIdx][lIdx].y_poly -= minExtent[1] ;
         }
      }

      // Setting base height for buildings if there is a DEM file
      if (WID->simParams->DTE_heightField && WID->simParams->DTE_mesh)
      {
         for (auto pIdx = 0; pIdx < WID->simParams->shpPolygons.size(); pIdx++)
         {
            // Get base height of every corner of building from terrain height
            min_height = WID->simParams->DTE_mesh->getHeight(WID->simParams->shpPolygons[pIdx][0].x_poly,
                                                             WID->simParams->shpPolygons[pIdx][0].y_poly);
            if (min_height < 0)
            {
               min_height = 0.0;
            }
            for (auto lIdx = 1; lIdx < WID->simParams->shpPolygons[pIdx].size(); lIdx++)
            {
               corner_height = WID->simParams->DTE_mesh->getHeight(WID->simParams->shpPolygons[pIdx][lIdx].x_poly,
                                                                   WID->simParams->shpPolygons[pIdx][lIdx].y_poly);

               if (corner_height < min_height && corner_height >= 0.0)
               {
                  min_height = corner_height;
               }
            }
            base_height.push_back(min_height);
         }
      }
      else
      {
         for (auto pIdx = 0; pIdx < WID->simParams->shpPolygons.size(); pIdx++)
         {
            base_height.push_back(0.0);
         }
      }

      for (auto pIdx = 0; pIdx < WID->simParams->shpPolygons.size(); pIdx++)
      {
         for (auto lIdx=0; lIdx < WID->simParams->shpPolygons[pIdx].size(); lIdx++)
         {
            WID->simParams->shpPolygons[pIdx][lIdx].x_poly += WID->simParams->halo_x;
            WID->simParams->shpPolygons[pIdx][lIdx].y_poly += WID->simParams->halo_y;
         }
      }

      std::cout << "Creating buildings from shapefile...\n";
      // Loop to create each of the polygon buildings read in from the shapefile
      for (auto pIdx = 0; pIdx < WID->simParams->shpPolygons.size(); pIdx++)
      {
         allBuildingsV.push_back (new PolyBuilding (WID, this, pIdx));
         building_id.push_back(allBuildingsV.size()-1);
         allBuildingsV[pIdx]->setPolyBuilding(this);
         allBuildingsV[pIdx]->setCellFlags(WID, this, pIdx);
         effective_height.push_back (allBuildingsV[pIdx]->height_eff);
      }
      std::cout << "\tdone.\n";

      auto buildingsetup_finish = std::chrono::high_resolution_clock::now();  // Finish recording execution time

      std::chrono::duration<float> elapsed_cut = buildingsetup_finish - buildingsetup_start;
      std::cout << "Elapsed time for building setup : " << elapsed_cut.count() << " s\n";

   }


   // SHP processing is done.  Now, consolidate all "buildings" onto
   // the same list...  this includes any canopies and building types
   // that were read in via the XML file...

   // Add all the Canopy* to it (they are derived from Building)
   if ( WID->canopies )
   {
      for (size_t i = 0; i < WID->canopies->canopies.size(); i++)
      {
         allBuildingsV.push_back( WID->canopies->canopies[i] );
         effective_height.push_back(allBuildingsV[i]->height_eff);
         building_id.push_back(allBuildingsV.size()-1);
      }
   }


   // Add all the Building* that were read in from XML to this list
   // too -- could be RectBuilding, PolyBuilding, whatever is derived
   // from Building in the end...
   if ( WID->buildings )
   {
      std::cout << "Consolidating building data..." << std::endl;

      float corner_height, min_height;
      for (size_t i = 0; i < WID->buildings->buildings.size(); i++)
      {
         allBuildingsV.push_back( WID->buildings->buildings[i] );
         int j = allBuildingsV.size()-1;
         building_id.push_back( j );
         // Setting base height for buildings if there is a DEM file
         if (WID->simParams->DTE_heightField && WID->simParams->DTE_mesh)
         {
            // Get base height of every corner of building from terrain height
            min_height = WID->simParams->DTE_mesh->getHeight(allBuildingsV[j]->polygonVertices[0].x_poly,
                                                             allBuildingsV[j]->polygonVertices[0].y_poly);
            if (min_height < 0)
            {
               min_height = 0.0;
            }
            for (size_t lIdx = 1; lIdx < allBuildingsV[j]->polygonVertices.size(); lIdx++)
            {
               corner_height = WID->simParams->DTE_mesh->getHeight(allBuildingsV[j]->polygonVertices[lIdx].x_poly,
                                                                   allBuildingsV[j]->polygonVertices[lIdx].y_poly);

               if (corner_height < min_height && corner_height >= 0.0)
               {
                  min_height = corner_height;
               }
            }
            allBuildingsV[j]->base_height = min_height;
         }
         else
         {
            allBuildingsV[j]->base_height = 0.0;
         }
         allBuildingsV[j]->setPolyBuilding(this);
         allBuildingsV[j]->setCellFlags(WID, this, j);
         effective_height.push_back(allBuildingsV[i]->height_eff);
      }
   }

   // We want to sort ALL buildings here...  use the allBuildingsV to
   // do this... (remember some are canopies) so we may need a
   // virtual function in the Building class to get the appropriate
   // data for the sort.
   std::cout << "Sorting buildings by height..." << std::endl;
   mergeSort( effective_height, allBuildingsV, building_id );

   wall = new Wall();

   std::cout << "Defining Solid Walls...\n";
   // Boundary condition for building edges
   wall->defineWalls(this);
   std::cout << "Walls Defined...\n";

   wall->solverCoefficients (this);

   /////////////////////////////////////////////////////////
   /////       Read coefficients from a file            ////
   /////////////////////////////////////////////////////////

   if (WID->simParams->readCoefficientsFlag == 1)
   {
     NCDFInput = new NetCDFInput(WID->simParams->coeffFile);

     start = {0,0,0,0};
     NCDFInput->getDimensionSize("x",ncnx);
     NCDFInput->getDimensionSize("y",ncny);
     NCDFInput->getDimensionSize("z",ncnz);
     NCDFInput->getDimensionSize("t",ncnt);

     count = {static_cast<unsigned long>(1),
             static_cast<unsigned long>(ncnz-1),
             static_cast<unsigned long>(ncny-1),
             static_cast<unsigned long>(ncnx-1)};


     NCDFInput->getVariableData("icellflag",start,count,icellflag);

     for (int k = 0; k < nz-2; k++)
     {
         for (int j = 0; j < ny-1; j++)
         {
             for (int i = 0; i < nx-1; i++)
             {
                 int icell_cent = i + j*(nx-1) + k*(nx-1)*(ny-1);
                 if (icellflag[icell_cent] != 0 && icellflag[icell_cent] != 2 && icellflag[icell_cent] != 8 && icellflag[icell_cent] != 7)
                 {
                   icellflag[icell_cent] = 1;
                 }
             }
         }
     }

     // Read in solver coefficients
     NCDFInput->getVariableData("e",start,count,e);
     NCDFInput->getVariableData("f",start,count,f);
     NCDFInput->getVariableData("g",start,count,g);
     NCDFInput->getVariableData("h",start,count,h);
     NCDFInput->getVariableData("m",start,count,m);
     NCDFInput->getVariableData("n",start,count,n);

   }

   // ///////////////////////////////////////
   // Generic Parameterization Related Stuff
   // ///////////////////////////////////////
   for (size_t i = 0; i < allBuildingsV.size(); i++)
   {
      // for now this does the canopy stuff for us
      allBuildingsV[building_id[i]]->canopyVegetation(this);
   }

   ///////////////////////////////////////////
   //   Upwind Cavity Parameterization     ///
   ///////////////////////////////////////////
   if (WID->simParams->upwindCavityFlag > 0)
   {
      std::cout << "Applying upwind cavity parameterization...\n";
      for (size_t i = 0; i < allBuildingsV.size(); i++)
      {
         allBuildingsV[building_id[i]]->upwindCavity(WID, this);
      }
      std::cout << "Upwind cavity parameterization done...\n";
   }

   //////////////////////////////////////////////////
   //   Far-Wake and Cavity Parameterizations     ///
   //////////////////////////////////////////////////
   if (WID->simParams->wakeFlag > 0)
   {
      std::cout << "Applying wake behind building parameterization...\n";
      for (size_t i = 0; i < allBuildingsV.size(); i++)
      {
         allBuildingsV[building_id[i]]->polygonWake(WID, this, building_id[i]);
      }
      std::cout << "Wake behind building parameterization done...\n";
   }

   ///////////////////////////////////////////
   //   Street Canyon Parameterization     ///
   ///////////////////////////////////////////
   if (WID->simParams->streetCanyonFlag > 0)
   {
      std::cout << "Applying street canyon parameterization...\n";
      for (size_t i = 0; i < allBuildingsV.size(); i++)
      {
         allBuildingsV[building_id[i]]->streetCanyon(this);
      }
      std::cout << "Street canyon parameterization done...\n";
   }

   ///////////////////////////////////////////
   //      Sidewall Parameterization       ///
   ///////////////////////////////////////////
   if (WID->simParams->sidewallFlag > 0)
   {
      std::cout << "Applying sidewall parameterization...\n";
      for (size_t i = 0; i < allBuildingsV.size(); i++)
      {
         allBuildingsV[building_id[i]]->sideWall(WID, this);
      }
      std::cout << "Sidewall parameterization done...\n";
   }


   ///////////////////////////////////////////
   //      Rooftop Parameterization        ///
   ///////////////////////////////////////////
   if (WID->simParams->rooftopFlag > 0)
   {
      std::cout << "Applying rooftop parameterization...\n";
      for (size_t i = 0; i < allBuildingsV.size(); i++)
      {
         allBuildingsV[building_id[i]]->rooftop (WID, this);
      }
      std::cout << "Rooftop parameterization done...\n";
   }

   wall->setVelocityZero (this);

   return;

}


void WINDSGeneralData::mergeSort( std::vector<float> &effective_height, std::vector<Building*> allBuildingsV, std::vector<int> &building_id)
{
   //if the size of the array is 1, it is already sorted
   if ( allBuildingsV.size() == 1)
   {
      return;
   }

   if ( allBuildingsV.size() > 1)
   {
      //make left and right sides of the data
      std::vector<float> effective_height_L, effective_height_R;
      std::vector<int> building_id_L, building_id_R;
      std::vector<Building*> allBuildingsV_L, allBuildingsV_R;
      effective_height_L.resize(allBuildingsV.size() / 2);
      effective_height_R.resize(allBuildingsV.size() - allBuildingsV.size() / 2);
      building_id_L.resize(allBuildingsV.size() / 2);
      building_id_R.resize(allBuildingsV.size() - allBuildingsV.size()/2);
      allBuildingsV_L.resize(allBuildingsV.size() / 2);
      allBuildingsV_R.resize(allBuildingsV.size() - allBuildingsV.size() / 2);

      //copy data from the main data set to the left and right children
      size_t lC = 0, rC = 0;
      for (size_t i = 0; i < allBuildingsV.size(); i++)
      {
         if (i < allBuildingsV.size() / 2)
         {
            effective_height_L[lC] = effective_height[i];
            allBuildingsV_L[lC] = allBuildingsV[i];
            building_id_L[lC++] = building_id[i];

         }
         else
         {
            effective_height_R[rC] = effective_height[i];
            allBuildingsV_R[rC] = allBuildingsV[i];
            building_id_R[rC++] = building_id[i];

         }
      }
      //recursively sort the children
      mergeSort( effective_height_L, allBuildingsV_L, building_id_L );
      mergeSort( effective_height_R, allBuildingsV_R, building_id_R );

      //compare the sorted children to place the data into the main array
      lC = rC = 0;
      for (size_t i = 0; i < allBuildingsV.size(); i++)
      {
         if (rC == effective_height_R.size() || ( lC != effective_height_L.size() &&
                                                  effective_height_L[lC] < effective_height_R[rC]))
         {
            effective_height[i] = effective_height_L[lC];
            building_id[i] = building_id_L[lC++];
         }
         else
         {
            effective_height[i] = effective_height_R[rC];
            building_id[i] = building_id_R[rC++];
         }
      }
   }

   return;
}




float WINDSGeneralData::canopyBisection(float ustar, float z0, float canopy_top, float canopy_atten, float vk, float psi_m)
{
   int iter;
   float  uhc, d, d1, d2;
   float tol, fnew, fi;

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


WINDSGeneralData::WINDSGeneralData()
{
}

WINDSGeneralData::~WINDSGeneralData()
{
}
