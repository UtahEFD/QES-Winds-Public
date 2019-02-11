/** \file "WRFInput.cpp" input data header file. 
    \author Pete Willemsen, Matthieu 

    Copyright (C) 2019 Pete Willemsen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/


#include "WRFInput.h"

WRFInput::WRFInput(const std::string& filename)
    : wrfInputFile( filename, NcFile::read )
{
    std::cout <<"there are "<<wrfInputFile.getVarCount()<<" variables"<<std::endl;;
    std::cout <<"there are "<<wrfInputFile.getAttCount()<<" attributes"<<std::endl;;

   std::cout <<"there are "<<wrfInputFile.getGroupCount()<<" groups"<<std::endl;;
   std::cout <<"there are "<<wrfInputFile.getTypeCount()<<" types"<<std::endl;;

   // Read in all the dm
   std::cout <<"there are "<< wrfInputFile.getDimCount()<<" dimensions"<<std::endl;;
   
}

WRFInput::~WRFInput()
{
}

void WRFInput::readDomainInfo()
{
    
// Read domain dimensions, terrain elevation, wind data, land-use and Z0 from WRF output. 
// Possibility to crop domain borders by adding a varargin (expected format: [Xstart,Xend;YStart,Yend]).
// Layers of data are recorder in the following format: (row,col) = (ny,nx).

//SimData.Clock = ncread(WRFFile,'Times'); % Time in vector format
                                                
    // Read the WRF NetCDF file as read-only (default option in the
    // following call).
//    try
//    {
        // Open the file for read access

        // The netCDF file is automatically closed by the NcFile destructor
    // wrfInputFile( "/scratch/Downloads/RXCwrfout_d07_2012-11-11_15-21", NcFile::read );

        // Retrieve the variable named "Times": char Times(Time, DateStrLen) ;
        NcVar simDataClock = wrfInputFile.getVar("Times");
        if (simDataClock.isNull()) return;

        // This is a string???
        // simDataClock.getVal

        std::cout << "Number of attributes: " << wrfInputFile.getAttCount() << std::endl;
        std::multimap<std::string,NcGroupAtt> globalAttributes = wrfInputFile.getAtts();
        // You can see all of the attributes with the following snippet:
        // for (auto i=globalAttributes.cbegin(); i!=globalAttributes.cend(); i++) {
        // std::cout << "Attribute Name: " << i->first << std::endl;
        // }
        
        auto gblAttIter = globalAttributes.find("WEST-EAST_GRID_DIMENSION");
        xDim[0] = 1;

        // Grab the Stored end of the dimension and subtract 1.
        // xDim+1 is a pointer reference to the 2nd array value.
        // Same happens for yDim below.
        gblAttIter->second.getValues( xDim+1 );
        xDim[1] -= 1;

        gblAttIter = globalAttributes.find("SOUTH-NORTH_GRID_DIMENSION");
        yDim[0] = 1;
        gblAttIter->second.getValues( yDim+1 );
        yDim[1] -= 1;

        std::cout << "XDim = " << xDim[0] << ", " << xDim[1] << std::endl;
        std::cout << "YDim = " << yDim[0] << ", " << yDim[1] << std::endl;

        // Compute nx and ny
        nx = xDim[1] - xDim[0] + 1;
        ny = yDim[1] - yDim[0] + 1;

        std::cout << "(nx, ny) = (" << nx << ", " << ny << ")" << std::endl;

        // Pull out DX and DY
        double cellSize[2];
        gblAttIter = globalAttributes.find("DX");
        gblAttIter->second.getValues( cellSize );

        gblAttIter = globalAttributes.find("DY");
        gblAttIter->second.getValues( cellSize+1 );
        std::cout << "DX = " << cellSize[0] << ", DY = " << cellSize[1] << std::endl;

           
// % If new domain borders are defined
// if nargin == 3 
        
//    NewDomainCorners = varargin{1};
    
//    XSTART_New = NewDomainCorners(1,1); XEND_New = NewDomainCorners(1,2); 
//    YSTART_New = NewDomainCorners(2,1); YEND_New = NewDomainCorners(2,2);
    
//    SimData.nx = XEND_New - XSTART_New +1;
//    SimData.ny = YEND_New - YSTART_New +1; 
    
//    SimData.OLD_XSTART = SimData.XSTART; SimData.OLD_XEND = SimData.XEND;
//    SimData.OLD_YSTART = SimData.YSTART; SimData.OLD_YEND = SimData.YEND;
        
//    SimData.XSTART = XSTART_New; SimData.XEND = XEND_New;
//    SimData.YSTART = YSTART_New; SimData.YEND = YEND_New;
      
// end


// Relief = ncread(WRFFile,'HGT');
// SimData.Relief = Relief(SimData.XSTART:SimData.XEND,SimData.YSTART:SimData.YEND,1)'; 

        //
        // Relief
        //
        NcVar reliefVar = wrfInputFile.getVar("HGT");
        std::cout << "relief dim count: " << reliefVar.getDimCount() << std::endl;
        std::vector<NcDim> dims = reliefVar.getDims();
        long totalDim = 1;
        for (int i=0; i<dims.size(); i++) {
            std::cout << "Dim: " << dims[i].getName() << ", ";
            if (dims[i].isUnlimited())
                std::cout << "Unlimited (" << dims[i].getSize() << ")" << std::endl;
            else
                std::cout << dims[i].getSize() << std::endl;
            totalDim *= dims[i].getSize();
        }
        std::cout << "relief att count: " << reliefVar.getAttCount() << std::endl;
        std::map<std::string, NcVarAtt> reliefVar_attrMap = reliefVar.getAtts();
        for (std::map<std::string, NcVarAtt>::const_iterator ci=reliefVar_attrMap.begin();
             ci!=reliefVar_attrMap.end(); ++ci) {
            std::cout << "Relief Attr: " << ci->first << std::endl;
    }

    // this is a 114 x 114 x 41 x 360 dim array...
    // slice by slice 114 x 114 per slice; 41 slices; 360 times

            // How do I extract out the start to end in and and y?
            // PHB = double(PHB(SimData.XSTART:SimData.XEND,
            // SimData.YSTART:SimData.YEND, :, SimData.TIMEVECT));

    // we want all slice data (xstart:xend X ystart:yend) at all
    // slices but only for the first 2 time series (TIMEVECT)
    double* reliefData = new double[ totalDim ];
    reliefVar.getVar( reliefData );
    
    // whole thing is in... now
    long subsetDim = nx * ny;
    double* reliefSave = new double[ subsetDim ];
    for (auto l=0; l<subsetDim; l++)
        reliefSave[l] = reliefData[l];
        
    std::cout << "first 10 for relief" << std::endl;
    for (auto l=0; l<10; l++)
        std::cout << reliefSave[l] << std::endl;

    std::cout << "last 10 for relief" << std::endl;
    for (auto l=subsetDim-10-1; l<subsetDim; l++)
        std::cout << reliefSave[l] << std::endl;

    delete [] reliefData;


    // % Wind data    
    // SimData = WindFunc(SimData); 
    readWindData();

    //
    // LU = ncread(WRFFile,'LU_INDEX');
    //
        NcVar LUIndexVar = wrfInputFile.getVar("LU_INDEX");
        std::cout << "LUIndex dim count: " << LUIndexVar.getDimCount() << std::endl;
        dims.clear();
        dims = LUIndexVar.getDims();
        totalDim = 1;
        for (int i=0; i<dims.size(); i++) {
            std::cout << "Dim: " << dims[i].getName() << ", ";
            if (dims[i].isUnlimited())
                std::cout << "Unlimited (" << dims[i].getSize() << ")" << std::endl;
            else
                std::cout << dims[i].getSize() << std::endl;
            totalDim *= dims[i].getSize();
        }
        std::cout << "LUIndex att count: " << LUIndexVar.getAttCount() << std::endl;
        std::map<std::string, NcVarAtt> LUIndexVar_attrMap = LUIndexVar.getAtts();
        for (std::map<std::string, NcVarAtt>::const_iterator ci=LUIndexVar_attrMap.begin();
             ci!=LUIndexVar_attrMap.end(); ++ci) {
            std::cout << "LUIndex Attr: " << ci->first << std::endl;
    }

    // this is a 114 x 114 x 41 x 360 dim array...
    // slice by slice 114 x 114 per slice; 41 slices; 360 times

            // How do I extract out the start to end in and and y?
            // PHB = double(PHB(SimData.XSTART:SimData.XEND,
            // SimData.YSTART:SimData.YEND, :, SimData.TIMEVECT));

    // we want all slice data (xstart:xend X ystart:yend) at all
    // slices but only for the first 2 time series (TIMEVECT)
    double* LUIndexData = new double[ totalDim ];
    LUIndexVar.getVar( LUIndexData );
    
    // whole thing is in... now
    subsetDim = nx * ny;
    double* LUIndexSave = new double[ subsetDim ];
    for (auto l=0; l<subsetDim; l++)
        LUIndexSave[l] = LUIndexData[l];
        
    std::cout << "first 10 for LUIndex" << std::endl;
    for (auto l=0; l<10; l++)
        std::cout << LUIndexSave[l] << std::endl;

    std::cout << "last 10 for LUIndex" << std::endl;
    for (auto l=subsetDim-10-1; l<subsetDim; l++)
        std::cout << LUIndexSave[l] << std::endl;

    delete [] LUIndexData;

//    }catch(NcException& e)
//     {
//       e.what();
//       cout<<"FAILURE*************************************"<<endl;
//       return;
//     }

}


void WRFInput::roughnessLength()
{
#if 0
% Computes a roughness length array covering each point of the grid
% In case 1 INPUT file must be WRF RESTART file
% In case 2 INPUT file must be WRF OUTPUT
% In case 3 INPUT variable is a constant value

switch Z0Flag
    
    case 1   %%% WRF RESTART file
        
        Z0 = ncread(SimData.Z0DataSource ,'Z0');
        Z0 = Z0(SimData.XSTART:SimData.XEND, SimData.YSTART:SimData.YEND)';
        
    case 2   %%% WRF OUTPUT file
        
        Z0 = zeros(size(SimData.LU,1),size(SimData.LU,2));
        
        for i = 1:size(SimData.LU,1)
            for j = 1:size(SimData.LU,2) 
                
                switch SimData.LU(i,j) 
                    
%%%%%%%%%%%%%%%%%%%%%%%%%     USGS LAND COVER     %%%%%%%%%%%%%%%%%%%%%%%%% 

%                     case 1   %%% Urban and built-up land
%                         Z0(i,j) = 0.8;
%                     case 2   %%% Dryland cropland and pasture
%                         Z0(i,j) = 0.15;
%                     case 3   %%% Irrigated cropland and pasture
%                         Z0(i,j) = 0.1;
%                     case 4   %%% Mixed dryland/irrigated cropland and pasture
%                         Z0(i,j) = 0.15;
%                     case 5   %%% Cropland/grassland mosaic
%                         Z0(i,j) = 0.14;
%                     case 6   %%% Cropland/woodland mosaic
%                         Z0(i,j) = 0.2;
%                     case 7   %%% Grassland
%                         Z0(i,j) = 0.12;
%                     case 8   %%% Shrubland
%                         Z0(i,j) = 0.05;
%                     case 9   %%% Mixed shrubland/grassland
%                         Z0(i,j) = 0.06;
%                     case 10   %%% Savanna
%                         Z0(i,j) = 0.15;
%                     case 11   %%% Deciduous broadleaf forest
%                         Z0(i,j) = 0.5;
%                     case 12   %%% Deciduous needleleaf forest
%                         Z0(i,j) = 0.5;
%                     case 13   %%% Evergreeen broadleaf forest
%                         Z0(i,j) = 0.5;
%                     case 14   %%% Evergreen needleleaf forest
%                         Z0(i,j) = 0.5;
%                     case 15   %%% Mixed forest
%                         Z0(i,j) = 0.5;
%                     case 16   %%% Water bodies
%                         Z0(i,j) = 0.0001;
%                     case 17   %%% Herbaceous wetland
%                         Z0(i,j) = 0.2;
%                     case 18   %%% Wooded wetland
%                         Z0(i,j) = 0.4;
%                     case 19   %%% Barren or sparsely vegetated
%                         Z0(i,j) = 0.01;
%                     case 20   %%% Herbaceous tundra
%                         Z0(i,j) = 0.1;
%                     case 21   %%% Wooded tundra
%                         Z0(i,j) = 0.3;
%                     case 22   %%% Mixed tundra
%                         Z0(i,j) = 0.15;
%                     case 23   %%% Bare ground tundra
%                         Z0(i,j) = 0.1;
%                     case 24   %%% Snow or ice
%                         Z0(i,j) = 0.001;

%%%%%%%%%%%%%%%%%%%%%%%%%      MODIS-WINTER      %%%%%%%%%%%%%%%%%%%%%%%%%%

                    case 1   %%% Evergreen needleleaf forest
                        Z0(i,j) = 0.5;
                    case 2   %%% Evergreeen broadleaf forest
                        Z0(i,j) = 0.5;
                    case 3   %%% Deciduous needleleaf forest
                        Z0(i,j) = 0.5;
                    case 4   %%% Deciduous broadleaf forest
                        Z0(i,j) = 0.5;
                    case 5   %%% Mixed forests
                        Z0(i,j) = 0.5;
                    case 6   %%% Closed Shrublands
                        Z0(i,j) = 0.1;
                    case 7   %%% Open Shrublands
                        Z0(i,j) = 0.1;
                    case 8   %%% Woody Savannas
                        Z0(i,j) = 0.15;
                    case 9   %%% Savannas
                        Z0(i,j) = 0.15;
                    case 10   %%% Grasslands
                        Z0(i,j) = 0.075;
                    case 11   %%% Permanent wetlands
                        Z0(i,j) = 0.3;
                    case 12   %%% Croplands
                        Z0(i,j) = 0.075;
                    case 13   %%% Urban and built-up land
                        Z0(i,j) = 0.5;
                    case 14   %%% Cropland/natural vegetation mosaic
                        Z0(i,j) = 0.065;
                    case 15   %%% Snow or ice
                        Z0(i,j) = 0.01;
                    case 16   %%% Barren or sparsely vegetated
                        Z0(i,j) = 0.065;
                    case 17   %%% Water
                        Z0(i,j) = 0.0001;
                    case 18   %%% Wooded tundra
                        Z0(i,j) = 0.15;
                    case 19   %%% Mixed tundra
                        Z0(i,j) = 0.1;
                    case 20   %%% Barren tundra
                        Z0(i,j) = 0.06;
                    case 21   %%% Lakes
                        Z0(i,j) = 0.0001;
                end
            end
        end
        
    case 3   %%% User-defined constant
        
        Z0 = repmat(SimData.Z0DataSource, SimData.XEND-SimData.XSTART+1, SimData.YEND-SimData.YSTART+1)';
end
    #endif
}



void WRFInput::readWindData()
{
    // This function computes velocity magnitude, direction and vertical
    // coordinates from WRF velocity components U,V and geopotential height.
    // Values are interpolated at each corresponding cell center.

    // Extraction of the Wind data vertical position
    NcVar phbVar = wrfInputFile.getVar("PHB");
    std::cout << "PHB dim count: " << phbVar.getDimCount() << std::endl;
    std::vector<NcDim> dims = phbVar.getDims();
    long totalDim = 1;
    for (int i=0; i<dims.size(); i++) {
        std::cout << "Dim: " << dims[i].getName() << ", ";
        if (dims[i].isUnlimited())
            std::cout << "Unlimited (" << dims[i].getSize() << ")" << std::endl;
        else
            std::cout << dims[i].getSize() << std::endl;
        totalDim *= dims[i].getSize();
    }
    std::cout << "PHB att count: " << phbVar.getAttCount() << std::endl;
    std::map<std::string, NcVarAtt> phbVar_attrMap = phbVar.getAtts();
    for (std::map<std::string, NcVarAtt>::const_iterator ci=phbVar_attrMap.begin();
         ci!=phbVar_attrMap.end(); ++ci) {
        std::cout << "PHB Attr: " << ci->first << std::endl;
    }

    // this is a 114 x 114 x 41 x 360 dim array...
    // slice by slice 114 x 114 per slice; 41 slices; 360 times

            // How do I extract out the start to end in and and y?
            // PHB = double(PHB(SimData.XSTART:SimData.XEND,
            // SimData.YSTART:SimData.YEND, :, SimData.TIMEVECT));

    // we want all slice data (xstart:xend X ystart:yend) at all
    // slices but only for the first 2 time series (TIMEVECT)
    double* phbData = new double[ totalDim ];
    phbVar.getVar( phbData );
    
    // whole thing is in... now
    int nz = 41;
    long subsetDim = nx * ny * nz * 2;
    double* phbSave = new double[ subsetDim ];
    for (auto l=0; l<subsetDim; l++)
        phbSave[l] = phbData[l];
        
//    for (long l=0; l<subsetDim; l++) {
//        std::cout << "d[" << l << "] = " << dSave[l] << std::endl;
//    }

    std::cout << "first 10 for phb" << std::endl;
    for (auto l=0; l<10; l++)
        std::cout << phbData[l] << std::endl;

    std::cout << "last 10 for phb" << std::endl;
    for (auto l=subsetDim-10-1; l<subsetDim; l++)
        std::cout << phbData[l] << std::endl;

    delete [] phbData;
    

    // Extraction of the Wind data vertical position
    NcVar phVar = wrfInputFile.getVar("PH");
    std::cout << "PH dim count: " << phVar.getDimCount() << std::endl;
    
    std::cout << "PH att count: " << phVar.getAttCount() << std::endl;
    std::map<std::string, NcVarAtt> phVar_attrMap = phVar.getAtts();
    for (std::map<std::string, NcVarAtt>::const_iterator ci=phVar_attrMap.begin();
         ci!=phVar_attrMap.end(); ++ci) {
        std::cout << "PH Attr: " << ci->first << std::endl;
    }

    // this is a 114 x 114 x 41 x 360 dim array...
    // slice by slice 114 x 114 per slice; 41 slices; 360 times

            // How do I extract out the start to end in and and y?
            // PHB = double(PHB(SimData.XSTART:SimData.XEND,
            // SimData.YSTART:SimData.YEND, :, SimData.TIMEVECT));

    // we want all slice data (xstart:xend X ystart:yend) at all
    // slices but only for the first 2 time series (TIMEVECT)
    double* phData = new double[ totalDim ];
    phVar.getVar( phData );
    
    // whole thing is in... now
    double* phSave = new double[ subsetDim ];
    for (auto l=0; l<subsetDim; l++)
        phSave[l] = phData[l];

    std::cout << "first 10 for ph" << std::endl;
    for (auto l=0; l<10; l++)
        std::cout << phData[l] << std::endl;
    
    std::cout << "last 10 for ph" << std::endl;
    for (auto l=subsetDim-10-1; l<subsetDim; l++)
        std::cout << phData[l] << std::endl;

    delete [] phData;


    // 
    /// Height
    // 

    double* heightData = new double[ subsetDim ];
    for (auto l=0; l<subsetDim; l++) {
        heightData[l] = (phbSave[l] + phSave[l]) / 9.81;
        // std::cout << "H[" << l << "] = " << heightData[l] << std::endl;
    }


    std::cout << "first 10 for height" << std::endl;
    for (auto l=0; l<10; l++) {
        std::cout << "H[" << l << "] = " << heightData[l] << std::endl;
    }


    std::cout << "last 10 for height" << std::endl;
    for (auto l=subsetDim-10-1; l<subsetDim; l++) {
        std::cout << "H[" << l << "] = " << heightData[l] << std::endl;
    }


#if 0

PHB = ncread(SimData.WRFFile,'PHB');
PHB = double(PHB(SimData.XSTART:SimData.XEND, SimData.YSTART:SimData.YEND, :, SimData.TIMEVECT));

PH = ncread(SimData.WRFFile,'PH');
PH = double(PH(SimData.XSTART:SimData.XEND, SimData.YSTART:SimData.YEND, :, SimData.TIMEVECT));

Height = (PHB + PH)./9.81; % Converting to meters
SimData.NbAlt = size(Height,3) - 1;
#endif


// Data seems good above here



#if 0

% Wind components
Ustagg = ncread(SimData.WRFFile,'U');
Ustagg = Ustagg(SimData.XSTART:SimData.XEND +1, SimData.YSTART:SimData.YEND, :, SimData.TIMEVECT);

Vstagg = ncread(SimData.WRFFile,'V');
Vstagg = Vstagg(SimData.XSTART:SimData.XEND, SimData.YSTART:SimData.YEND +1, :, SimData.TIMEVECT);


%% Centering values %%

SimData.NbAlt = size(Height,3) - 1;

U = zeros(SimData.nx,SimData.ny,SimData.NbAlt,numel(SimData.TIMEVECT));
for x = 1:SimData.nx
    U(x,:,:,:) = .5*(Ustagg(x,:,:,:) + Ustagg(x+1,:,:,:));
end

V = zeros(SimData.nx,SimData.ny,SimData.NbAlt,numel(SimData.TIMEVECT));
for y = 1:SimData.ny   
    V(:,y,:,:) = .5*(Vstagg(:,y,:,:) + Vstagg(:,y+1,:,:));
end

SimData.CoordZ = zeros(SimData.nx,SimData.ny,SimData.NbAlt,numel(SimData.TIMEVECT));
for k = 1:SimData.NbAlt
    SimData.CoordZ(:,:,k,:) = .5*(Height(:,:,k,:) + Height(:,:,k+1,:));
end

%% Velocity and direction %%

SimData.WS = sqrt(U.^2 + V.^2);

SimData.WD = zeros(SimData.nx,SimData.ny,SimData.NbAlt,numel(SimData.TIMEVECT));

for x = 1:SimData.nx
    for y = 1:SimData.ny
        for Alt = 1:SimData.NbAlt
            for Time = 1:numel(SimData.TIMEVECT)
                if U(x,y,Alt,Time) > 0
                    SimData.WD(x,y,Alt,Time) = 270-(180/pi)*atan(V(x,y,Alt,Time)/U(x,y,Alt,Time));
                else
                    SimData.WD(x,y,Alt,Time) = 90-(180/pi)*atan(V(x,y,Alt,Time)/U(x,y,Alt,Time));
                end
            end
        end
    end
end

%% Permutation to set dimensions as (row,col,etc) = (ny,nx,nz,nt) %%

SimData.WD = permute(SimData.WD,[2 1 3 4]);
SimData.WS = permute(SimData.WS,[2 1 3 4]);
SimData.CoordZ = permute(SimData.CoordZ,[2 1 3 4]);
#endif

}

void WRFInput::setWRFDataPoint()
{
#if 0
% If MaxNbStat is smaller than the number of WRF data point available, then
% we operate a selection. Vertical height is selected between defined boundaries.
% For each stations, their horizontal and vertical coordinates, wind speed 
% and direction, along with the number of vertical altitude pts, are recorded 

if SimData.nx*SimData.ny > SimData.MaxNbStat
    
    nx2 = sqrt(SimData.MaxNbStat*SimData.nx/SimData.ny);
    ny2 = sqrt(SimData.MaxNbStat*SimData.ny/SimData.nx);
    
    WRF_RowY = (1:SimData.ny/ny2:SimData.ny);
    WRF_ColX = (1:SimData.nx/nx2:SimData.nx);
    
    WRF_RowY = unique(round(WRF_RowY));
    WRF_ColX = unique(round(WRF_ColX));
    
else
    
    WRF_RowY = (1:SimData.ny);
    WRF_ColX = (1:SimData.nx);
    
end

SimData.NbStat = numel(WRF_RowY)*numel(WRF_ColX);
StatData.CoordX = zeros(1,SimData.NbStat);
StatData.CoordY = zeros(1,SimData.NbStat);
StatData.nz = zeros(1,SimData.NbStat);

StatData.CoordZ = struct([]);
StatData.WS = struct([]);
StatData.WD = struct([]);


Stat = 1;
for y = WRF_RowY
    for x = WRF_ColX
        
        StatData.CoordX(Stat) = x;
        StatData.CoordY(Stat) = y;
        
        levelk_max = 0;
        for t = 1:numel(SimData.TIMEVECT)                     
            CoordZ_xyt = SimData.CoordZ(y,x,:,t);        
            [levelk] = find(CoordZ_xyt >= SimData.MinWRFAlt & CoordZ_xyt <= SimData.MaxWRFAlt);
            if numel(levelk) > numel(levelk_max)
                levelk_max = levelk; % If wind data heights change during time, higher height vector is selected
            end
        end
        
        StatData.CoordZ{Stat} = reshape(SimData.CoordZ(y,x,levelk_max,:),numel(levelk_max),size(SimData.CoordZ,4));
        StatData.nz(Stat) = size(StatData.CoordZ{Stat},1);
        
        StatData.WS{Stat} = reshape(SimData.WS(y,x,levelk_max,:),numel(levelk_max),size(SimData.WS,4));
        StatData.WD{Stat} = reshape(SimData.WD(y,x,levelk_max,:),numel(levelk_max),size(SimData.WD,4));   
        
        Stat = Stat + 1;
    end
end

SimData.maxCoordz = 0; % Will be used to set domain vertical dimension
SimData.minCoordz = 0;
for i = 1:SimData.NbStat
    SimData.minCoordz = min(min(min(SimData.maxCoordz,StatData.CoordZ{i})));
    SimData.maxCoordz = max(max(max(SimData.maxCoordz,StatData.CoordZ{i})));
end

fprintf('%i %s %g %s %g %s\n',SimData.NbStat,' WRF data points have been generated between ',SimData.minCoordz,' and ',SimData.maxCoordz,' meters AGL')
#endif


}

