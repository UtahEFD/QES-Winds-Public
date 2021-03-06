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
 * This is a template class that holds 3 values. These values
 * can be accessed as if this was an array.
 */

#include <type_traits>
#include <iostream>
#include "util/ParseInterface.h"


#define FLOATS_ARE_EQUAL(x,y) ( ( (x) - (y)) < 0.000001 && ( (x) - (y)) >   -0.000001 )

template <class T>
class Vector3 : public ParseInterface
{
protected:
	std::vector<T> values;
public:

	Vector3()
	{
            values.clear();
            values.push_back( (0) );
            values.push_back( (0) );
            values.push_back( (0) );
	}

/*	template <typename X> Vector3(const Vector3<X>& newV)
	{
		for (int i = 0; i < 3; i++)
			values[i] = newV[i];
	}
*/

	template <typename X> Vector3(const X a, const X b, const X c)
	{
            values.clear();
            values.push_back(a);
            values.push_back(b);
            values.push_back(c);
	}

	virtual void parseValues()
	{
		values.clear();
		parseTaglessValues<T>(values);
	}

	/*
	 * accesses the value at position i
	 *
	 * @param i -the index of the value to return
	 * @return a reference to the value stored at i
	 */
	T& operator[](const int i)
	{
		return values[i%3];
	}

	/*
	 * returns if two Vector3 values of the same type are equal
	 *
	 * @param v -the vector3 to compare with this
	 * @return if values at index 0,1,2 are all equal with their counterparts
	 */
	bool operator==(const Vector3<T>& v)
	{
		if (std::is_same<T,float>::value || std::is_same<T,double>::value)
			return FLOATS_ARE_EQUAL(values[0], v.values[0]) && FLOATS_ARE_EQUAL(values[1], v.values[1]) && FLOATS_ARE_EQUAL(values[2], v.values[2]);
		else
			return v.values[0] == values[0] && v.values[1] == values[1] && v.values[2] == values[2];
	}

        /*Vector3<T>& operator=(const Vector3<T>& v)
	{
		for (int i = 0; i < 3; i++)
			values[0] = v.values[i];
		return *this;
	}*/


	friend std::istream& operator>> (std::istream& is, Vector3<T>& v)
	{
	    is >> v.values[0] >> v.values[1] >> v.values[2];
	    return is;
	}

        friend Vector3<T> operator-(const Vector3<T>& v1, const Vector3<T>& v2){
           return Vector3<T> (v1.values[0] - v2.values[0], v1.values[1] - v2.values[1], v1.values[2] - v2.values[2]);
        }
};
