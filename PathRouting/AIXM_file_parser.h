#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <exception>
#include <pugixml.hpp>
#define _USE_MATH_DEFINES
#include <QtDebug>

class QuadTree;

struct Coordinate
{
  double m_Lat,m_Lon,m_X,m_Y;
 Coordinate():m_Lat(0.0), m_Lon(0), m_X(0), m_Y(0)
  {
  }

  enum CompareType{ Lat, Lon, X, Y };
  CompareType m_type;

  double operator()( const CompareType& p_type )
  {
    switch (p_type)
    {
      case Coordinate::Lat:
        return m_Lat;
        break;
      case Coordinate::Lon:
        return m_Lon;
        break;
      case Coordinate::X:
        return m_X;
        break;
      case Coordinate::Y:
        return m_Y;
        break;
      default:
        return m_X;
    };
  }

  void set_geocoord( const double& p_Lon, const double& p_Lat )
  {
    m_Lat = p_Lat;
    m_Lon = p_Lon;
  }	
};


struct Coordinate_compare
{
 Coordinate_compare( const Coordinate::CompareType& p_type): m_type(p_type) {}
	
  bool operator()( const Coordinate& p_coord_1 , const Coordinate& p_coord_2 )
  {
    switch (m_type)
    {
      case Coordinate::Lat:
        return p_coord_1.m_Lat <p_coord_2.m_Lat;
        break;
      case Coordinate::Lon:
        return p_coord_1.m_Lon < p_coord_2.m_Lon;
        break;
      case Coordinate::X:
        return p_coord_1.m_X < p_coord_2.m_X;
        break;
      case Coordinate::Y:
        return p_coord_1.m_Y < p_coord_2.m_Y;
        break;
      default:
        return p_coord_1.m_X < p_coord_2.m_X;
    };
    return false;
  }

  Coordinate::CompareType m_type;
};

class AIXM_file_parser
{
public:
  AIXM_file_parser(void);
  void link_to_QuadTree(std::unique_ptr<QuadTree>& p_Quadtree);
  ~AIXM_file_parser(void);

  double polarTodec( std::string polarCoord );
  float mapDistance ( float dLat, float dLon );
  double x_distance( double lon1, double lat1 );
  double y_distance( double lon1, double lat1 );
  bool read_AIXM_file( std::string full_path );
  void find_boundaries();

  struct GMLObject
  {
    std::string m_AIXM_object_type;
    std::vector< Coordinate > m_Coordinates;
  };

  std::vector< GMLObject* > Objects;

  void adjust_position( GMLObject* );

  std::unique_ptr<QuadTree> m_Quadtree;		// Link to Quadtree object
};


struct simple_walker: pugi::xml_tree_walker
{
  simple_walker();
simple_walker(AIXM_file_parser& p_parser):
  parser( p_parser )
  {
  }
  
  AIXM_file_parser& parser;
  AIXM_file_parser::GMLObject* object;

  virtual bool for_each(pugi::xml_node& node);
};


double distance( double p_lon1, double p_lat1, double p_lon2, double p_lat2 );
double bearing_rad( double p_lon1, double p_lat1, double p_lon2, double p_lat2 );
