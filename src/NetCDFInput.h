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


//
//  Input.hpp
//  
//  This class handles netcdf input
//
//  Created by Jeremy Gibbs on 03/15/19.
//  Modified by Fabien Margairaz

#pragma once

#include <string>
#include <vector>
#include <map>
#include <netcdf>

/**
 * This class handles reading input files.
 */

using namespace netCDF;
using namespace netCDF::exceptions;

class NetCDFInput {
    
 protected:
  // netCDF variables
  NcFile* infile;
  
 public:
    
  // initializer
  NetCDFInput()
    {}
  NetCDFInput(std::string);
  virtual ~NetCDFInput()
    {}
  
  // getters
  void getDimension(std::string, NcDim&);
  void getDimensionSize(std::string name, int&);
  void getVariable(std::string, NcVar&);
  
  // get variable for 1D 
  void getVariableData(std::string name,std::vector<int>&);
  void getVariableData(std::string name,std::vector<float>&);
  void getVariableData(std::string name,std::vector<double>&);
  
  // get variable for *D
  void getVariableData(std::string name,std::vector<size_t>,std::vector<size_t>,std::vector<int>&);
  void getVariableData(std::string name,std::vector<size_t>,std::vector<size_t>,std::vector<float>&);
  void getVariableData(std::string name,std::vector<size_t>,std::vector<size_t>,std::vector<double>&);

};

