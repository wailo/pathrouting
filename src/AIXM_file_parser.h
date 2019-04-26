#ifndef AIXM_FILE_PARSER_H
#define AIXM_FILE_PARSER_H

#include "airport.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <pugixml.hpp>
#include <regex>
#include <sstream>
#include <string>
#define _USE_MATH_DEFINES
#include <QtDebug>

class QuadTree;

class AIXM_file_parser {
public:
  AIXM_file_parser(void);
  ~AIXM_file_parser(void);

  bool read_AIXM_file(const std::string &full_path, airport &airport);

  AIXM_coordinate m_max_coord, m_min_coord;
};

double distance(double p_lon1, double p_lat1, double p_lon2, double p_lat2);
double bearing_rad(double p_lon1, double p_lat1, double p_lon2, double p_lat2);

#endif /* AIXM_FILE_PARSER_H */
