#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <vector>

class point{
public:
	//coordinates of points in domain
	double x;
	double y;
	//order of points in domain
	unsigned long long Id;

	//Constructors
	point();
	point(double xInput, double yInput, unsigned long long id);
	point(double xInput, double yInput);
	point(const point &pInput);

	// Operations
	void set(point pInput);
	void setX(double xInput);
	void setY(double yInput);

	void setId(unsigned long long id);
	unsigned long long getId();

	double getX();
	double getY();
	bool operator == (point p);
};

inline std::ostream &operator << (std::ostream &str, point const &p){
	return str << "Point id: " << p.Id << " x= "<<p.x<<" y="<<p.y<<std::endl;
}

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

inline optional<point> getMinPoint(std::vector<point> const & points)
{
        if (points.empty())
                return optional<point>();

        point min = points[0]; 
        for (unsigned i = 0; i < points.size(); ++i) { 
                if (points[i].x < min.x)
                        min.x = points[i].x;
                if (points[i].y < min.y)
                        min.y = points[i].y;
        }
        return optional<point>(min);
}

inline optional<point> getMaxPoint(std::vector<point> const & points)
{
        if (points.empty())
                return optional<point>();

        point max = points[0];  
        for (unsigned i = 0; i < points.size(); ++i) { 
                if (points[i].x > max.x)
                        max.x = points[i].x;
                if (points[i].y > max.y)
                        max.y = points[i].y;
        }
        return optional<point>(max);
}


#endif

