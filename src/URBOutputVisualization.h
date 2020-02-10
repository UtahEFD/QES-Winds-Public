#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "URBGeneralData.h"
#include "URBInputData.h"
#include "NetCDFOutputGeneric.h"

/* Specialized output classes derived from NetCDFOutputGeneric for 
   cell center data (used primarly for vizualization)
*/
class URBOutputVisualization : public NetCDFOutputGeneric
{
public:
  URBOutputVisualization()
    : NetCDFOutputGeneric()
  {}
  URBOutputVisualization(URBGeneralData*,URBInputData*,std::string);
  ~URBOutputVisualization()	       
  {}
  
  void save(float);
  
protected:
  bool validateFileOtions();
  
private:
  std::vector<float> x_out,y_out,z_out;
  std::vector<int> icellflag_out;
  std::vector<double> u_out,v_out,w_out;

  URBGeneralData* ugd_;
  
  // all possible output fields need to be add to this list
  std::vector<std::string> allOutputFields = {"t","x","y","z","u","v","w","icell","terrain"};

};
