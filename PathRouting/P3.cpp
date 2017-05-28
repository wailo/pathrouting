#include "P3.h"


P3::P3(void):x(0), y(0), z(0)
{

}

P3::P3( const double p_X, const double p_Y, const double p_Z ):x(p_X), y(p_Y), z(p_Z)
{
}


P3::~P3(void)
{
}

P3 P3::operator+( const P3& other ) const
{
	P3 result;

	result.x = x + other.x;
	result.y = y + other.y;
	result.z = z + other.z;

	return result;
}

P3 P3::operator-( const P3& other ) const
{
	P3 result;

	result.x = x - other.x;
	result.y = y - other.y;
	result.z = z - other.z;

	return result;
}