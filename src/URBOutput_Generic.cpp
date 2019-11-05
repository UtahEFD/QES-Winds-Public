#include "URBOutput_Generic.h"

URBOutput_Generic::URBOutput_Generic(std::string output_file) 
  : NetCDFOutput(output_file)
{
};


void URBOutput_Generic::createDimensions(std::vector<NcDim> dim_vect)
{
  // scalar dimension 
  dim_scalar_t.push_back(dim_vect[0]);
  dim_scalar_z.push_back(dim_vect[1]);
  dim_scalar_y.push_back(dim_vect[2]);
  dim_scalar_x.push_back(dim_vect[3]);
  
  // 3D vector dimension (time dep)
  dim_vector.push_back(dim_vect[0]);
  dim_vector.push_back(dim_vect[1]);
  dim_vector.push_back(dim_vect[2]);
  dim_vector.push_back(dim_vect[3]);
  
  // 2D vector (surface, indep of time)
  dim_vector_2d.push_back(dim_vect[2]);
  dim_vector_2d.push_back(dim_vect[3]);
  
}

//----------------------------------------
// create attribute scalar
// -> int
void URBOutput_Generic::createAttScalar(std::string name,
					std::string long_name,
					std::string units,
					std::vector<NcDim> dims,
					int* data)
{
  // FM -> here I do not know what is the best way to add the ref to data.
  AttScalarInt att = {data,name,long_name,units,dims};
  map_att_scalar_int.emplace(name,att);
  

}
// -> float
void URBOutput_Generic::createAttScalar(std::string name,
					std::string long_name,
					std::string units,
					std::vector<NcDim> dims,
					float* data)
{
  // FM -> here I do not know what is the best way to add the ref to data.
  AttScalarFlt att = {data,name,long_name,units,dims};
  map_att_scalar_flt.emplace(name,att);
  
}
// -> double
void URBOutput_Generic::createAttScalar(std::string name,
					std::string long_name,
					std::string units,
					std::vector<NcDim> dims,
					double* data)
{
  // FM -> here I do not know what is the best way to add the ref to data.
  AttScalarDbl att = {data,name,long_name,units,dims};
  map_att_scalar_dbl.emplace(name,att);
}

//----------------------------------------
// create attribute Vector
// -> int
void URBOutput_Generic::createAttVector(std::string name,
					std::string long_name,
					std::string units,
					std::vector<NcDim> dims,
					std::vector<int>* data)
{
  // FM -> here I do not know what is the best way to add the ref to data.
  AttVectorInt att = {data,name,long_name,units,dims};
  map_att_vector_int.emplace(name,att);
  
}
// -> float
void URBOutput_Generic::createAttVector(std::string name,
					std::string long_name,
					std::string units,
					std::vector<NcDim> dims,
					std::vector<float>* data)
{
  // FM -> here I do not know what is the best way to add the ref to data.
  AttVectorFlt att = {data,name,long_name,units,dims};
  map_att_vector_flt.emplace(name,att);
  
}
// -> double
void URBOutput_Generic::createAttVector(std::string name,
					std::string long_name,
					std::string units,
					std::vector<NcDim> dims,
					std::vector<double>* data)
{
  // FM -> here I do not know what is the best way to add the ref to data.
  AttVectorDbl att = {data,name,long_name,units,dims};
  map_att_vector_dbl.emplace(name,att);
}

//----------------------------------------
void URBOutput_Generic::addOutputFields()
{
   /*
    This function add the  fields to the output vectors
    and link them to the NetCDF.

    Since the type is not know, one needs to loop through 
    the 6 output vector to find it.
        
    FMargairaz
  */
  
  // create list of fields to save base on output_fields
  for (size_t i=0; i<output_fields.size(); i++) {
    std::string key = output_fields[i];
        
    if (map_att_scalar_int.count(key)) {
      // scalar int
      output_scalar_int.push_back(map_att_scalar_int[key]);
    } else if (map_att_scalar_flt.count(key)) {
      // scalar flt
      output_scalar_flt.push_back(map_att_scalar_flt[key]);
    } else if (map_att_scalar_dbl.count(key)) {
      // scalar dbl
      output_scalar_dbl.push_back(map_att_scalar_dbl[key]);  
    } else if(map_att_vector_int.count(key)) {
      // vector int
      output_vector_int.push_back(map_att_vector_int[key]);
    } else if(map_att_vector_flt.count(key)) {
      // vector flt
      output_vector_flt.push_back(map_att_vector_flt[key]);    
    } else if (map_att_vector_dbl.count(key)) {
      // vector dbl
      output_vector_dbl.push_back(map_att_vector_dbl[key]);
    }
  }

  // add scalar fields
  // -> int
  for ( AttScalarInt att : output_scalar_int ) {
    addField(att.name, att.units, att.long_name, att.dimensions, ncInt);
  }  
  // -> float
  for ( AttScalarFlt att : output_scalar_flt ) {    
    addField(att.name, att.units, att.long_name, att.dimensions, ncFloat);
  }
  // -> double
  for ( AttScalarDbl att : output_scalar_dbl ) {
    addField(att.name, att.units, att.long_name, att.dimensions, ncDouble);
  }
  // add vector fields
  // -> int
  for ( AttVectorInt att : output_vector_int ) {
    addField(att.name, att.units, att.long_name, att.dimensions, ncInt);
  }
  // -> int
  for ( AttVectorFlt att : output_vector_flt ) {
    addField(att.name, att.units, att.long_name, att.dimensions, ncFloat);
  }
  // -> double
  for ( AttVectorDbl att : output_vector_dbl ) {
    addField(att.name, att.units, att.long_name, att.dimensions, ncDouble);
  }
  
};


void URBOutput_Generic::rmOutputField(std::string name)
{
  /*
    This function remove a field from the output vectors
    Since the type is not know, one needs to loop through 
    the 6 output vector to find it.
    
    Note: the field CANNOT be added again.
    
    FMargairaz
  */

  // loop through scalar fields to remove
  // -> int
  for (unsigned int i=0; i<output_scalar_int.size(); i++) {
    if(output_scalar_int[i].name==name) {
      output_scalar_int.erase(output_scalar_int.begin()+i);
      return;
    }
  }
  // -> float
  for (unsigned int i=0; i<output_scalar_flt.size(); i++) {
    if(output_scalar_flt[i].name==name) {
      output_scalar_flt.erase(output_scalar_flt.begin()+i);
      return;
    } 
  }
  
  // -> double
  for (unsigned int i=0; i<output_scalar_dbl.size(); i++) {
    if(output_scalar_dbl[i].name==name) {
      output_scalar_dbl.erase(output_scalar_dbl.begin()+i);
      return;
    } 
  }
  
  // loop through vector fields to remove
  // -> int
  for (unsigned int i=0; i<output_vector_int.size(); i++) {
    if(output_vector_int[i].name==name) {
      output_vector_int.erase(output_vector_int.begin()+i);
      return;
    } 
  }
  // -> float
  for (unsigned int i=0; i<output_vector_flt.size(); i++) { 
    if(output_vector_flt[i].name==name) {
      output_vector_flt.erase(output_vector_flt.begin()+i);
      return;
    } 
  }
  // -> double
  for (unsigned int i=0; i<output_vector_dbl.size(); i++) {
    if(output_vector_dbl[i].name==name) {
      output_vector_dbl.erase(output_vector_dbl.begin()+i);
      return;
    } 
  } 
};

void URBOutput_Generic::saveOutputFields()
{
   /*
    This function save the fields from the output vectors
    Since the type is not know, one needs to loop through 
    the 6 output vector to find it.
    
    FMargairaz
   */

  // loop through scalar fields to save
  // -> int
  for (unsigned int i=0; i<output_scalar_int.size(); i++) {
    std::vector<size_t> scalar_index;
    scalar_index = {static_cast<unsigned long>(output_counter)};  
    saveField1D(output_scalar_int[i].name, scalar_index,
		output_scalar_int[i].data);
  }
  // -> float
  for (unsigned int i=0; i<output_scalar_flt.size(); i++) {
    std::vector<size_t> scalar_index;
    scalar_index = {static_cast<unsigned long>(output_counter)}; 
    saveField1D(output_scalar_flt[i].name, scalar_index,
		output_scalar_flt[i].data);
  }
  // -> double
  for (unsigned int i=0; i<output_scalar_dbl.size(); i++) {
    std::vector<size_t> scalar_index;
    scalar_index = {static_cast<unsigned long>(output_counter)}; 
    saveField1D(output_scalar_dbl[i].name, scalar_index,
		output_scalar_dbl[i].data);
  }
  
  // loop through vector fields to save
  // -> int
  for (unsigned int i=0; i<output_vector_int.size(); i++) {
    // 1D (x or z or y)
    if (output_vector_int[i].dimensions.size()==1) {
      saveField2D(output_vector_int[i].name, *output_vector_int[i].data);
    } 
    // 2D (terrain)
    else if (output_vector_int[i].dimensions.size()==2) {
      std::vector<size_t> vector_index;
      std::vector<size_t> vector_size;
      int ny=output_vector_int[i].dimensions[0].getSize();
      int nx=output_vector_int[i].dimensions[1].getSize();

      vector_index = {0, 0};
      vector_size = {static_cast<unsigned long>(ny),
			 static_cast<unsigned long>(nx)};

      saveField2D(output_vector_int[i].name, vector_index,
		  vector_size, *output_vector_int[i].data);
    }
    // 4D (u,v,w)
    else if (output_vector_int[i].dimensions.size()==4) {
      std::vector<size_t> vector_index;
      std::vector<size_t> vector_size;
      int nz=output_vector_int[i].dimensions[1].getSize();
      int ny=output_vector_int[i].dimensions[2].getSize();
      int nx=output_vector_int[i].dimensions[3].getSize();
      
      vector_index = {static_cast<size_t>(output_counter), 0, 0, 0};
      vector_size  = {1, static_cast<unsigned long>(nz),
		      static_cast<unsigned long>(ny),
		      static_cast<unsigned long>(nx)};
      
      saveField2D(output_vector_int[i].name, vector_index,
		  vector_size, *output_vector_int[i].data);
    }
  }
  // -> float
  for (unsigned int i=0; i<output_vector_flt.size(); i++) { 
    // 1D (x or z or y)
    if (output_vector_flt[i].dimensions.size()==1) {
      saveField2D(output_vector_flt[i].name, *output_vector_flt[i].data);
    } 
    // 2D (terrain)
    else if (output_vector_flt[i].dimensions.size()==2) {
      std::vector<size_t> vector_index;
      std::vector<size_t> vector_size;
      int ny=output_vector_flt[i].dimensions[0].getSize();
      int nx=output_vector_flt[i].dimensions[1].getSize();
      
      vector_index = {0, 0};
      vector_size = {static_cast<unsigned long>(ny),
		     static_cast<unsigned long>(nx)};
      saveField2D(output_vector_flt[i].name, vector_index,
		  vector_size, *output_vector_flt[i].data);
    }
    // 4D (u,v,w)
    else if (output_vector_flt[i].dimensions.size()==4) {
      
      std::vector<size_t> vector_index;
      std::vector<size_t> vector_size;
      int nz=output_vector_flt[i].dimensions[1].getSize();
      int ny=output_vector_flt[i].dimensions[2].getSize();
      int nx=output_vector_flt[i].dimensions[3].getSize();
      
      vector_index = {static_cast<size_t>(output_counter), 0, 0, 0};
      vector_size  = {1, static_cast<unsigned long>(nz),
		      static_cast<unsigned long>(ny),
		      static_cast<unsigned long>(nx)};
      
      saveField2D(output_vector_flt[i].name, vector_index,
		  vector_size, *output_vector_flt[i].data);
    }
  }
  // -> double
  for (unsigned int i=0; i<output_vector_dbl.size(); i++) {

    // 1D (x or z or y)
    if (output_vector_dbl[i].dimensions.size()==1) {
      saveField2D(output_vector_dbl[i].name, *output_vector_dbl[i].data);
    } 
    // 2D (terrain)
    else if (output_vector_dbl[i].dimensions.size()==2) {
      
      std::vector<size_t> vector_index;
      std::vector<size_t> vector_size;
      int ny=output_vector_dbl[i].dimensions[0].getSize();
      int nx=output_vector_dbl[i].dimensions[1].getSize();
      
      vector_index = {0, 0};
      vector_size  = {static_cast<unsigned long>(ny),
		      static_cast<unsigned long>(nx)};
      
      saveField2D(output_vector_dbl[i].name, vector_index,
		  vector_size, *output_vector_dbl[i].data);
    }
    // 4D (u,v,w)
    else if (output_vector_dbl[i].dimensions.size()==4) {
      std::vector<size_t> vector_index;
      std::vector<size_t> vector_size;
      int nz=output_vector_dbl[i].dimensions[1].getSize();
      int ny=output_vector_dbl[i].dimensions[2].getSize();
      int nx=output_vector_dbl[i].dimensions[3].getSize();
      
      vector_index = {static_cast<size_t>(output_counter), 0, 0, 0};
      vector_size  = {1, static_cast<unsigned long>(nz),
		      static_cast<unsigned long>(ny),
		      static_cast<unsigned long>(nx)};
      
      saveField2D(output_vector_dbl[i].name, vector_index,
		  vector_size, *output_vector_dbl[i].data);
    }
  }

};

