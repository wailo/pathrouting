#pragma once

// XYZ Point
class P3
{
public:

	double x;
	double y;
	double z;

	// Overloading + operator
	P3 operator+( const P3& other ) const;

	// Overloading - operator
	P3 operator-( const P3& other ) const;

	P3(void);
	P3( const double p_X, const double p_Y, const double p_Z ); 
	~P3(void);
};

