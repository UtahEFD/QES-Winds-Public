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


#pragma once

#include <cassert>
#include <vector>
#include "gdal.h"
#include "ogrsf_frmts.h"
#include <limits>

#include "PolygonVertex.h"

class ESRIShapefile
{
public:
    ESRIShapefile();
    ESRIShapefile(const std::string &filename, const std::string &layerName,
                  std::vector< std::vector< polyVert > >& polygons, std::vector <float> &building_height, float heightFactor);
    ~ESRIShapefile();

    void getLocalDomain( std::vector<float> &dim )
    {
        assert(dim.size() == 2);
        dim[0] = (int)ceil(maxBound[0] - minBound[0]);
        dim[1] = (int)ceil(maxBound[1] - minBound[1]);
    }

    void getMinExtent( std::vector<float> &ext )
    {
        assert(ext.size() == 2);
        ext[0] = minBound[0];
        ext[1] = minBound[1];
    }

private:

    void loadVectorData( std::vector< std::vector< polyVert > > &polygons, std::vector <float> &building_height, float heightFactor );

    std::string m_filename;
    std::string m_layerName;

    GDALDataset *m_poDS;

    std::vector<float> minBound, maxBound;
};
