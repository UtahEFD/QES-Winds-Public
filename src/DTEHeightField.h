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


#ifndef __DTE_HEIGHT_FIELD_H__
#define __DTE_HEIGHT_FIELD_H__ 1


#include <string>
#include "Triangle.h"
#include "Vector3.h"

#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "ogrsf_frmts.h"

#include "Cell.h"
#include "Edge.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <chrono>

class DTEHeightField
{
public:

  friend class test_DTEHeightField;

  DTEHeightField();
  DTEHeightField(const std::string &filename, double cellSizeXN, double cellSizeYN, float UTMx, float UTMy, int OriginFlag, float DEMDistanceX, float DEMDistanceY, int nx, int ny);

    DTEHeightField(const std::vector<double> &heightField, int dimX, int dimY, double cellSizeXN, double cellSizeYN);

  ~DTEHeightField();

  std::vector<Triangle*> getTris() const {return m_triList;}

  /*
   * This function takes in a domain to change and a grid size for
   * the size of sells in the domain. This iterates over all triangles
   * and shifts all points so that the minimum value on each axis
   * is 0. This will then use the greatest point divided by the size
   * of one sell in the grid to set the value of the given domain.
   *
   * @param domain -The domain that will be changed to match the dem file
   * @param grid -The size of each cell in the domain space.
   */
  void setDomain(Vector3<int>* domain, Vector3<float>* grid);


  /*
   * This function takes the triangle list that represents the dem file and
   * outputs the mesh in an obj file format to the file "s"
   *
   * @param s -The file that the obj data will be written to.
   */
  void outputOBJ(std::string s);

  /*
   * This function takes in a list of cells, and the domain space and queries
   * the height field at corners of the each cell setting coordinates and the
   * substances present in each cell. This then returns a list of ints that are
   * the id's of all cut-cells(cells that are both terrain and air).
   *
   * @param cells -The list of cells to be initialized
   * @param nx -X dimension in the domain
   * @param ny -Y dimension in the domain
   * @param nz -Z dimension in the domain
   * @param dx -size of a cell in the X axis
   * @param dy -size of a cell in the Y axis
   * @param dz -size of a cell in the Z axis
   * @return -A list of ID values for all cut cells.
   */
  std::vector<int> setCells(Cell* cells, int nx, int ny, int nz, float dx, float dy,
                            std::vector<float> &dz_array, std::vector<float> z_face,
                            float halo_x, float halo_y) const;

  /*
   * This function frees the pafScanline. It should be called after all DEM querying has taken place.
   */
  void closeScanner();

    void convertRasterToGeo( double rasterX, double rasterY, double &geoX, double &geoY )
    {
        // Affine transformation from the GDAL geotransform:
        // https://gdal.org/user/raster_data_model.html
        geoX = m_geoTransform[0] + rasterX * m_geoTransform[1] + rasterY * m_geoTransform[2];
        geoY = m_geoTransform[3] + rasterX * m_geoTransform[4] + rasterY * m_geoTransform[5];
    }
    

  int m_nXSize, m_nYSize;
  float pixelSizeX, pixelSizeY;
  int originFlag;
  float DEMDistancex, DEMDistancey;
  double adfMinMax[2];

private:

  /*
   * This function is given the height of the DEM file at each of it's corners and uses
   * them to calculate at what points cells are intersected by the quad the corners form.
   * the cells are then updated to reflect the cut.
   * @param cells -The list of cells to be initialized
   * @param i -The current x dimension index of the cell
   * @param i -The current y dimension index of the cell
   * @param nx -X dimension in the domain
   * @param ny -Y dimension in the domain
   * @param nz -Z dimension in the domain
   * @param dz -size of a cell in the Z axis
   * @param corners -an array containing the points that representing the DEM elevation at each of the cells corners
   * @param cutCells -a list of all cells which the terrain goes through
   */
  void setCellPoints(Cell* cells, int i, int j, int nx, int ny, int nz, std::vector<float> &dz_array, std::vector<float> z_face, Vector3<float> corners[], std::vector<int>& cutCells) const;

  void load();

  void printProgress (float percentage);

  // void loadImage();

  bool compareEquality( double f1, double f2 ) const
  {
    const double eps = 1.0e-6;
    return fabs( f1 - f2 ) < eps;
  }

  float queryHeight( float *scanline, int j, int k )  const
  {
    float height;
    if ( j >= m_nXSize || k >= m_nYSize)
    {
      height = 0.0;
    }
    //if (j * m_nXSize + k >= m_nXSize * m_nYSize
    else
    {
        // important to remember range is [0, n-1], so need the -1
        // in the flip
        // Previous code had this -- does not seem correct
        // height = scanline[ abs(k-m_nYSize) * m_nXSize + j ];
        height = scanline[ (m_nYSize-1 - k) * (m_nXSize) + j ] -adfMinMax[0]; 
    }

    if (height < 0.0 || std::isnan(abs(height)))
    {
      height = 0.0;
    }

    if (!compareEquality( height, m_rbNoData ))
    {
      height = height * m_rbScale + m_rbOffset;
    }
    else
    {
      height = m_rbMin;
    }


    return height;
  }

  /*
   * Given a height between the two points (z value) this function will create
   * a third point which exists on the line from a to b existing at height h.
   * in the result that a and b exist on the same height, the mid point between
   * the two will be returned instead.
   *
   * @param a -first point designating the line
   * @param b -second point designating the line
   * @param height -the height at which the third point will be created
   * @return -an intermediate point existing on the line from a to b at z value height
   */
  Vector3<float> getIntermediate(Vector3<float> a, Vector3<float> b, float height) const;



  std::string m_filename;
  GDALDataset  *m_poDataset;
  double m_geoTransform[6];

  //int m_nXSize, m_nYSize;
  double m_rbScale, m_rbOffset, m_rbNoData, m_rbMin;

  // Texture relative information
  GDALDataset  *m_imageDataset;
  int m_imageXSize, m_imageYSize;
  double m_imageGeoTransform[6];

  //float pixelSizeX, pixelSizeY;
  float cellSizeX, cellSizeY;
  float domain_UTMx, domain_UTMy;
  float origin_x, origin_y;
  int shift_x = 0;
  int shift_y = 0;
  int domain_nx, domain_ny;
  int end_x = 0;
  int end_y = 0;

  std::vector<Triangle*> m_triList;
  float min[3], max[3];

  float *pafScanline;

};


#endif
