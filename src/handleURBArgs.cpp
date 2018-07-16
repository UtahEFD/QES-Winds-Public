#include "handleURBArgs.h"

URBArgs::URBArgs()
	: verbose(false), quicFile(""), cellFace(false), solveType(1)
{
    reg("help", "help/usage information", ArgumentParsing::NONE, '?');
    reg("verbose", "turn on verbose output", ArgumentParsing::NONE, 'v');
    reg("cellface", "select cellface, if not then cell center", ArgumentParsing::NONE, 'c');
    reg("solvetype", "selects the method for solving the windfield", ArgumentParsing::INT, 's');
    reg("quicproj", "Specifies the QUIC Proj file", ArgumentParsing::STRING, 'q');
    reg("netcdfout", "Specifies the netcdf file to write results to", ArgumentParsing::STRING, 'o');
    reg("demfile", "Specifies the DEM file that should be used for terrain", ArgumentParsing::STRING, 'd');
    reg("icellout", "Specifies that the iCellFlag values should be output", ArgumentParsing::NONE, 'i');
    reg("terrainout", "Specifies that the triangle mesh for the terrain should be output", ArgumentParsing::NONE, 't');
    reg("windsolveroff", "Turns off the wind solver and wind output", ArgumentParsing::NONE, 'x');
}

void URBArgs::processArguments(int argc, char *argv[])
{
    processCommandLineArgs(argc, argv);

    // Process the command line arguments after registering which
    // arguments you wish to parse.
    if (isSet("help")) {
        printUsage();
        exit(EXIT_SUCCESS);
    }
    
    verbose = isSet("verbose");
    if (verbose) std::cout << "Verbose Output: ON" << std::endl;
    
    isSet( "quicproj", quicFile );
    if (quicFile != "") std::cout << "quicproj set to " << quicFile << std::endl;

    isSet( "netcdfout", netCDFFile );
    if (netCDFFile != "") std::cout << "netCDF output file set to " << netCDFFile << std::endl;

    cellFace = isSet("cellface");
    if (cellFace) std::cout << "Cell face computations: ON" << std::endl;

    iCellOut = isSet("icellout");
    if (iCellOut) std::cout << "iCellFlag values WILL be output to iCellValues.nc" << std::endl;

    terrainOut = isSet("terrainout");
    if (terrainOut) std::cout << "the terrain triangle mesh WILL be output to terrain.obj" << std::endl;

    solveWind = isSet("windsolveroff");
    if (solveWind) std::cout << "the wind fields are not being calculated" << std::endl;

    isSet("solvetype", solveType);
    if (solveType == CPU_Type) std::cout << "Solving with: CPU" << std::endl;

    isSet( "demfile", demFile );
    if (demFile != "") std::cout << "DEM input file set to " << demFile << std::endl;
}