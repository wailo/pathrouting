#ifndef GML_OBJECT_H
#define GML_OBJECT_H

#include <string>
#include <vector>

struct AIXM_coordinate {
  double m_Lat, m_Lon;
  AIXM_coordinate() : m_Lat(0.0), m_Lon(0) {}

  enum CompareType { Lat, Lon };
  CompareType m_type;

  double operator()(const CompareType &p_type) {
    switch (p_type) {
    case AIXM_coordinate::Lat:
      return m_Lat;
      break;
    case AIXM_coordinate::Lon:
      return m_Lon;
      break;
    default:
      return m_Lon;
    };
  }
};

struct GMLObject {
  std::string m_AIXM_object_id;
  std::string m_AIXM_object_type;
  std::string m_GML_vertex_type;
  std::vector<AIXM_coordinate> m_Coordinates;
};

#endif /* GML_OBJECT_H */
