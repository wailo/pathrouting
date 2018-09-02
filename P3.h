#pragma once

// XYZ Point
class P3 {
public:
  double x, y, z;

  // Overloading + operator
  P3 operator+(const P3 &other) const;

  // Overloading - operator
  P3 operator-(const P3 &other) const;

  // Default Constructor
  P3(void);

  P3(const double p_X, const double p_Y, const double p_Z);

  // Default Destructor
  ~P3(void);
};
