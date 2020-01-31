#include "URBOutputVisualization.h"

URBOutputVisualization::URBOutputVisualization(URBGeneralData *ugd,URBInputData* uid,std::string output_file)
  : NetCDFOutputGeneric(output_file)
{
  std::cout<<"Getting output fields for Vizualization file"<<std::endl;

  std::vector<std::string> fileOP= uid->fileOptions->outputFields;
  bool valid_output;

  if (fileOP.empty() || fileOP[0]=="all") {
    output_fields = allOutputFields;
    valid_output=true;
  }else{
    output_fields={"t","x","y","z"};
    output_fields.insert(output_fields.end(),fileOP.begin(),fileOP.end());
    valid_output=validateFileOtions();
  }

  if(!valid_output){
     std::cerr << "Error: invalid output fields for visulization fields output\n";
     exit(EXIT_FAILURE);
  }

  // copy of ugd pointer
  ugd_=ugd;
  
  
  int nx = ugd_->nx;
  int ny = ugd_->ny;
  int nz = ugd_->nz;

  long numcell_cout = (nx-1)*(ny-1)*(nz-2);

  z_out.resize( nz-2 );
  for (auto k=1; k<nz-1; k++) {
    z_out[k-1] = ugd_->z[k]; // Location of face centers in z-dir
  }

  x_out.resize( nx-1 );
  for (auto i=0; i<nx-1; i++) {
    x_out[i] = (i+0.5)*ugd_->dx; // Location of face centers in x-dir
  }

  y_out.resize( ny-1 );
  for (auto j=0; j<ny-1; j++) {
    y_out[j] = (j+0.5)*ugd_->dy; // Location of face centers in y-dir
  }

  // Output related data
  u_out.resize( numcell_cout, 0.0 );
  v_out.resize( numcell_cout, 0.0 );
  w_out.resize( numcell_cout, 0.0 );
  icellflag_out.resize( numcell_cout, 0.0 );

  // set cell-centered data dimensions
  // time dimension
  NcDim NcDim_t=addDimension("t");
  // space dimensions
  NcDim NcDim_x=addDimension("x",ugd_->nx-1);
  NcDim NcDim_y=addDimension("y",ugd_->ny-1);
  NcDim NcDim_z=addDimension("z",ugd_->nz-2);

  // create attributes for time dimension
  std::vector<NcDim> dim_vect_t;
  dim_vect_t.push_back(NcDim_t);
  createAttScalar("t","time","s",dim_vect_t,&time);

  // create attributes space dimensions
  std::vector<NcDim> dim_vect_x;
  dim_vect_x.push_back(NcDim_x);
  createAttVector("x","x-distance","m",dim_vect_x,&x_out);
  std::vector<NcDim> dim_vect_y;
  dim_vect_y.push_back(NcDim_y);
  createAttVector("y","y-distance","m",dim_vect_y,&y_out);
  std::vector<NcDim> dim_vect_z;
  dim_vect_z.push_back(NcDim_z);
  createAttVector("z","z-distance","m",dim_vect_z,&z_out);

  // create 2D vector (time indep)
  std::vector<NcDim> dim_vect_2d;
  dim_vect_2d.push_back(NcDim_y);
  dim_vect_2d.push_back(NcDim_x);
  // create attributes
  createAttVector("terrain","terrain height","m",dim_vect_2d,&(ugd_->terrain));

  // create 3D vector (time dep)
  std::vector<NcDim> dim_vect_3d;
  dim_vect_3d.push_back(NcDim_t);
  dim_vect_3d.push_back(NcDim_z);
  dim_vect_3d.push_back(NcDim_y);
  dim_vect_3d.push_back(NcDim_x);
  // create attributes
  createAttVector("u","x-component velocity","m s-1",dim_vect_3d,&u_out);
  createAttVector("v","y-component velocity","m s-1",dim_vect_3d,&v_out);
  createAttVector("w","z-component velocity","m s-1",dim_vect_3d,&w_out);
  createAttVector("icell","icell flag value","--",dim_vect_3d,&icellflag_out);

  // create output fields
  addOutputFields();

}

bool URBOutputVisualization::validateFileOtions()
{
  
  // check if all fileOptions->outputFields are possible
  bool doContains(true);
  std::size_t iter = 0, maxiter = output_fields.size();
  
  while(doContains && iter<maxiter) {
    doContains = find(allOutputFields.begin(),allOutputFields.end(),
                  output_fields.at(iter)) != allOutputFields.end();
    iter++;
  }
  
  return doContains;
}


// Save output at cell-centered values
void URBOutputVisualization::save(float timeOut)
{
  // get grid size (not output var size)
  int nx = ugd_->nx;
  int ny = ugd_->ny;
  int nz = ugd_->nz;

  // set time
  time = (double)timeOut;

  // get cell-centered values
  for (auto k = 1; k < nz-1; k++) {
    for (auto j = 0; j < ny-1; j++) {
      for (auto i = 0; i < nx-1; i++) {
        int icell_face = i + j*nx + k*nx*ny;
        int icell_cent = i + j*(nx-1) + (k-1)*(nx-1)*(ny-1);
        u_out[icell_cent] = 0.5*(ugd_->u[icell_face+1]+ugd_->u[icell_face]);
        v_out[icell_cent] = 0.5*(ugd_->v[icell_face+nx]+ugd_->v[icell_face]);
        w_out[icell_cent] = 0.5*(ugd_->w[icell_face+nx*ny]+ugd_->w[icell_face]);
        icellflag_out[icell_cent] = ugd_->icellflag[icell_cent+((nx-1)*(ny-1))];
      }
    }
  }
  
  // save the fields to NetCDF files
  saveOutputFields();
  
  // remove x, y, z and terrain
  // from output array after first save
  if (output_counter==0) {
    rmTimeIndepFields();
  }

  // increment for next time insertion
  output_counter +=1;
};
