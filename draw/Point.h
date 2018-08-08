
/*******************************************************************************
*  The "New BSD License" : http://www.opensource.org/licenses/bsd-license.php  *
********************************************************************************

Copyright (c) 2010, Mark Turney
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#ifndef POINT_H
#define POINT_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <iostream>


// taken from simple_svg 

// Quick optional return type.  This allows functions to return an invalid
//  value if no good return is possible.  The user checks for validity
//  before using the returned value.
template <typename T>
class optional
{
public:
	optional<T>(T const & type)
		: valid(true), type(type) { }
	optional<T>() : valid(false), type(T()) { }
	T * operator->()
	{
		// If we try to access an invalid value, an exception is thrown.
		if (!valid)
			throw std::exception();

		return &type;
	}
	// Test for validity.
	bool operator!() const { return !valid; }
private:
	bool valid;
	T type;
};



struct Point
{
	Point(double x = 0, double y = 0) : x(x), y(y) { }
	Point(const Point &p) : x(p.x), y(p.y) { }
	double x;
	double y;
};

inline optional<Point> getMinPoint(std::vector<Point> const & points)
{
	if (points.empty())
		return optional<Point>();

	Point min = points[1]; // skipping the 0th point
	for (unsigned i = 1; i < points.size(); ++i) { 
		if (points[i].x < min.x)
			min.x = points[i].x;
		if (points[i].y < min.y)
			min.y = points[i].y;
	}
	return optional<Point>(min);
}


inline optional<Point> getMaxPoint(std::vector<Point> const & points)
{
	if (points.empty())
		return optional<Point>();

	Point max = points[1];  // skipping the 0th point
	for (unsigned i = 1; i < points.size(); ++i) { 
		if (points[i].x > max.x)
			max.x = points[i].x;
		if (points[i].y > max.y)
			max.y = points[i].y;
	}
	return optional<Point>(max);
}

 
#endif
