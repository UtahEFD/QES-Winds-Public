#pragma once

/*
 * This function contains variables that define information
 * necessary for running the simulation.
 */

#include <string>
#include "util/ParseInterface.h"
#include "Vector3.h"
#include "DTEHeightField.h"
#include "ESRIShapefile.h"
#include "WRFInput.h"
#include "Mesh.h"

class SimulationParameters : public ParseInterface
{
private:


public:
    Vector3<int>* domain;
    Vector3<float>* grid;
    int verticalStretching;
    std::vector<float> dz_value;
    int totalTimeIncrements;
    int rooftopFlag;
    int upwindCavityFlag;
    int streetCanyonFlag;
    int streetIntersectionFlag;
    int wakeFlag;
    int sidewallFlag;
    int maxIterations;
    int residualReduction;
    float domainRotation;
    float UTMx;
    float UTMy;
    int UTMZone;
    int UTMZoneLetter;
    int meshTypeFlag;
    float halo_x = 0.0;
    float halo_y = 0.0;
    float heightFactor = 1.0;

    // DTE - digital elevation model details
    std::string demFile;    // DEM file name
    DTEHeightField* DTE_heightField = nullptr;
    Mesh* DTE_mesh;

    // SHP File parameters
    std::string shpFile;   // SHP file name
    std::string shpBuildingLayerName;
    ESRIShapefile *SHPData = nullptr;
    std::vector< std::vector <polyVert> > shpPolygons;
    std::vector <float> shpBuildingHeight;        // Height of
                                                  // buildings

    // //////////////////////////////////////////
    // WRF File Parameters
    // //////////////////////////////////////////
    // 
    // Two use-cases are now supported:
    //
    // (1) Only a WRF file is supplied.
    // If only a WRF data output file is supplied in the XML, the
    // elevation, terrain model is acquired from the WRF Fire Mesh.
    // Metparmas related to stations/sensors are pulled from the wind
    // profile supplied by WRF.
    // * Issues:
    // - We are not yet checking if no fire mesh is specified.  If no
    // fire mesh is available, the terrain height could come from the
    // atmos mesh.
    //
    // (2) Both a DEM and a WRF file are supplied.  With both a DEM
    // and WRF file, the DEM will be used for creating the terrain and
    // the WRF file will only be used to extract stations/sensors
    // pulled from the wind profile in the WRF atmospheric mesh. Thus,
    // no terrain will be queried from the WRF file.
    //
    
    std::string wrfFile;
    WRFInput *wrfInputData = nullptr;

    enum DomainInputType {
        WRFOnly,
        WRFDEM,
        DEMOnly,
        UNKNOWN
    };
    DomainInputType m_domIType;

    SimulationParameters()
    {
        UTMx = 0.0;
        UTMy = 0.0;
        UTMZone = 0;
        UTMZoneLetter = 0;
    }

    ~SimulationParameters()
    {
        // close the scanner
        if (DTE_heightField)
            DTE_heightField->closeScanner();
    }


    virtual void parseValues()
    {
        parseElement< Vector3<int> >(false, domain, "domain");   // when
                                                                 // parseElement
                                                                 // isn't
                                                                 // called,
                                                                 // how
                                                                 // does
                                                                 // this
                                                                 // get allocated?
        parseElement< Vector3<float> >(false, grid, "cellSize");
        parsePrimitive<int>(true, verticalStretching, "verticalStretching");
        parseMultiPrimitives<float>(false, dz_value, "dz_value");
        parsePrimitive<int>(false, totalTimeIncrements, "totalTimeIncrements");
        parsePrimitive<int>(true, rooftopFlag, "rooftopFlag");
        parsePrimitive<int>(true, upwindCavityFlag, "upwindCavityFlag");
        parsePrimitive<int>(true, streetCanyonFlag, "streetCanyonFlag");
        parsePrimitive<int>(true, streetIntersectionFlag, "streetIntersectionFlag");
        parsePrimitive<int>(true, wakeFlag, "wakeFlag");
        parsePrimitive<int>(true, sidewallFlag, "sidewallFlag");
        parsePrimitive<int>(true, maxIterations, "maxIterations");
        parsePrimitive<int>(true, residualReduction, "residualReduction");
        parsePrimitive<int>(true, meshTypeFlag, "meshTypeFlag");
        parsePrimitive<float>(true, domainRotation, "domainRotation");
        parsePrimitive<float>(false, UTMx, "UTMx");
        parsePrimitive<float>(false, UTMy, "UTMy");
        parsePrimitive<int>(false, UTMZone, "UTMZone");
        parsePrimitive<int>(false, UTMZoneLetter, "UTMZoneLetter");
        parsePrimitive<float>(false, halo_x, "halo_x");
        parsePrimitive<float>(false, halo_y, "halo_y");
        parsePrimitive<float>(false, heightFactor, "heightFactor");

        demFile = "";
        parsePrimitive<std::string>(false, demFile, "DEM");

        shpFile = "";
        parsePrimitive<std::string>(false, shpFile, "SHP");

        wrfFile = "";
        parsePrimitive<std::string>(false, wrfFile, "WRF");

        shpBuildingLayerName = "buildings";  // defaults
        parsePrimitive<std::string>(false, shpBuildingLayerName, "SHPBuildingLayer");

        // Determine which use case to use for WRF/DEM combinations
        if ((demFile != "") && (wrfFile != "")) {
            // Both specified
            // DEM - read in terrain
            // WRF - retrieve wind profiles only
            m_domIType = WRFDEM;
        }
        else if ((demFile == "") && (wrfFile != "")) {
            // Only WRF Specified
            // WRF - pull terrain and retrieve wind profiles
            m_domIType = WRFOnly;
        }
        else if (demFile != "") {
            // Only DEM Specified - nothing set for WRF Input
            m_domIType = DEMOnly;
        }
        else {
            m_domIType = UNKNOWN;
        }

        //
        // Process the data files based on the state determined above
        //
        if (m_domIType == WRFOnly) {
            //
            // WRF File is specified 
            // Read in height field
            //
            std::cout << "Processing WRF data for terrain and met param sensors from " << wrfFile << std::endl;
            wrfInputData = new WRFInput( wrfFile, UTMx, UTMy, 0, 0 );
            std::cout << "WRF Input Data processing completed." << std::endl;

            // In the current setup, grid may NOT be set... be careful
            // may need to initialize it here if nullptr is true for grid

            // utilize the wrf information to construct a
            // DTE_heightfield
            std::cout << "Constructing DTE from WRF Input" << std::endl;
            DTE_heightField = new DTEHeightField(wrfInputData->fmHeight,
                                                 wrfInputData->fm_nx,
                                                 wrfInputData->fm_ny,
                                                 (*(grid))[0],
                                                 (*(grid))[1]);

            // domain = new Vector3<int>( wrfInputData->fm_nx, wrfInputData->fm_nx, 100 );
            DTE_heightField->setDomain(domain, grid);
            DTE_mesh = new Mesh(DTE_heightField->getTris());
            std::cout << "Mesh complete\n";
        }
        else if (m_domIType == WRFDEM) {

            std::cout << "Reading DEM and processing WRF data for met param sensors from " << wrfFile << std::endl;

            // First read DEM as usual
            std::cout << "Extracting Digital Elevation Data from " << demFile << std::endl;
            DTE_heightField = new DTEHeightField(demFile,
                                                 (*(grid))[0],
                                                 (*(grid))[1] );
            assert(DTE_heightField);

            std::cout << "Forming triangle mesh...\n";
            DTE_heightField->setDomain(domain, grid);
            DTE_mesh = new Mesh(DTE_heightField->getTris());
            std::cout << "Mesh complete\n";

            // To proceed and cull sensors appropriately, we will need
            // the lower-left bounds from the DEM if UTMx and UTMy and
            // UTMZone are not all 0
            // 
            // ??? Parse primitive should return true or false if a
            // value was parsed, rather than this
            // ??? can refactor that later.
            bool useUTM_for_DEMLocation = false;
            float uEps = 0.001;
            if (((UTMx > -uEps) && (UTMx < uEps)) &&
                ((UTMy > -uEps) && (UTMy < uEps)) &&
                (UTMZone == 0)) {
                useUTM_for_DEMLocation = true;
                std::cout << "UTM (" << UTMx << ", " << UTMy << "), Zone: " << UTMZone << " will be used as lower-left location for DEM." << std::endl;
            }
            
            // Note from Pete:
            // Normally, will want to see if UTMx and UTMy are valid
            // in the DEM and if so, pass that UTMx and UTMy into the
            // WRFInput.  Passing 0s for these should not likely cause
            // issues since we're either adding or subtracting these
            // values.

            // Then, read WRF File extracting ONLY the sensor data
            bool onlySensorData = true;
            float dimX = (*(domain))[0] * (*(grid))[0];
            float dimY = (*(domain))[1] * (*(grid))[1];
            std::cout << "dimX = " << dimX << ", dimY = " << dimY << std::endl;
            wrfInputData = new WRFInput( wrfFile, UTMx, UTMy, dimX, dimY, onlySensorData );
            std::cout << "WRF Wind Velocity Profile Data processing completed." << std::endl;
        }
        
        else if (m_domIType == DEMOnly) {
            std::cout << "Extracting Digital Elevation Data from " << demFile << std::endl;
            DTE_heightField = new DTEHeightField(demFile,
                                                 (*(grid))[0],
                                                 (*(grid))[1] );
            assert(DTE_heightField);

            std::cout << "Forming triangle mesh...\n";
            DTE_heightField->setDomain(domain, grid);
            DTE_mesh = new Mesh(DTE_heightField->getTris());
            std::cout << "Mesh complete\n";
        }
        else {
            // No DEM, so make sure these are null
            DTE_heightField = nullptr;
            DTE_mesh = nullptr;
            wrfInputData = nullptr;
        }


        //
        // Process ESRIShapeFile here, but leave extraction of poly
        // building for later in URBGeneralData
        //
        SHPData = nullptr;
        if (shpFile != "") {

            // Read polygon node coordinates and building height from shapefile
            SHPData = new ESRIShapefile( shpFile, shpBuildingLayerName,
                                         shpPolygons, shpBuildingHeight, heightFactor );
        }
    }
};
