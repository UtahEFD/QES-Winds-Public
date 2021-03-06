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

/*
 * This clas represents a triangle being made up of 3 points each
 * with an x,y,z location
 */

#include "util/ParseInterface.h"
#include "Vector3.h"
#include <cmath>


#define LOWEST_OF_THREE(x,y,z) ( (x) <= (y) && (x) <= (z) ? (x) : ( (y) <= (x) && (y) <= (z) ? (y) : (z) ) )
#define HIGHEST_OF_THREE(x,y,z) ( (x) >= (y) && (x) >= (z) ? (x) : ( (y) >= (x) && (y) >= (z) ? (y) : (z) ) )

class Triangle : public ParseInterface
{
public:
	Vector3<float> *a, *b, *c;

	Triangle()
	{
		a = b = c = 0;
	}

	Triangle(Vector3<float> aN, Vector3<float> bN, Vector3<float> cN)
	{
		a = new Vector3<float>(aN);
		b = new Vector3<float>(bN);
		c = new Vector3<float>(cN);
	}

	/*
	 * uses a vertical ray cast from point x y at height 0 with barycentric interpolation to
	 * determine if the ray hits inside this triangle.
	 *
	 * @param x -x location
	 * @param y -y location
	 * @return the length of the ray before intersection, if no intersection, -1 is returned
	 */
	float getHeightTo(float x, float y);


	/*
	 * gets the minimum and maximum values in the x y and z dimensions
	 *
	 * @param xmin -lowest value in the x dimension
	 * @param xmax -highest value in the x dimension
	 * @param ymin -lowest value in the y dimension
	 * @param ymax -highest value in the y dimension
	 * @param zmin -lowest value in the z dimension
	 * @param zmax -highest value in the z dimension
	 */
	void getBoundaries(float& xmin, float& xmax, float& ymin, float& ymax, float& zmin, float& zmax);


	virtual void parseValues();
};
