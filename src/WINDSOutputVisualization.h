#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "WINDSGeneralData.h"
#include "WINDSInputData.h"
#include "QESNetCDFOutput.h"

/* Specialized output classes derived from QESNetCDFOutput for
   cell center data (used primarly for vizualization)
*/
class WINDSOutputVisualization : public QESNetCDFOutput
{
public:
  WINDSOutputVisualization()
    : QESNetCDFOutput()
  {}
  WINDSOutputVisualization(WINDSGeneralData*,WINDSInputData*,std::string);
  ~WINDSOutputVisualization()
  {}

  void save(float);

protected:
  bool validateFileOtions();

private:
  std::vector<float> x_out,y_out,z_out;
  std::vector<int> icellflag_out;
  std::vector<double> u_out,v_out,w_out;

  WINDSGeneralData* WGD_;

  // all possible output fields need to be add to this list
  std::vector<std::string> allOutputFields = {"t","x","y","z","u","v","w","icell","terrain"};

};
