#include "AIXM_file_parser.h"
#include "QuadTree.h"

#include <functional>
#include <math.h>

AIXM_file_parser::AIXM_file_parser(void) {
  m_min_coord.m_Lat = std::numeric_limits<double>::max();
  m_min_coord.m_Lon = std::numeric_limits<double>::max();
  m_max_coord.m_Lat = std::numeric_limits<double>::lowest();
  m_max_coord.m_Lon = std::numeric_limits<double>::lowest();
}

AIXM_file_parser::~AIXM_file_parser(void) {}

bool AIXM_file_parser::read_AIXM_file(const std::string &full_path, airport &airport) {
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(full_path.c_str());

  if (!result) {
    throw std::runtime_error("AIXM file does not exist:" + full_path);
  }

  const double DEG_TO_RAD = M_PI / 180.0;
  AIXM_coordinate min_coord, max_coord;

  const auto &aixm_members = doc.select_nodes("//message:hasMember", 0);
  for (const auto &member_itr : aixm_members) {

    const auto &node = member_itr.node();
    if (!strcmp(node.name(), "message:hasMember")) {

      GMLObject gml_object;
      std::string temp_str = std::string(node.first_child().name());
      std::size_t pos = temp_str.find_first_of(':');
      temp_str = temp_str.substr(pos + 1);
      gml_object.m_AIXM_object_type = temp_str;
      const auto identifier = node.select_node(pugi::xpath_query(".//gml:identifier", 0)).node().first_child().value();
      gml_object.m_AIXM_object_id = identifier;
      // Get the first parent which describes the vertices type
      temp_str = node.select_node(pugi::xpath_query(".//gml:posList/..", 0)).node().name();
      pos = temp_str.find_first_of(':');
      temp_str = temp_str.substr(pos + 1);
      gml_object.m_GML_vertex_type = temp_str;
      // Get the positions lists.
      const auto &pos_list_itr = node.select_node(pugi::xpath_query(".//gml:posList", 0));

      if (!pos_list_itr) {
        return false;
      }

      const auto &pos_list_node = pos_list_itr.node();

      std::istringstream iss(pos_list_node.first_child().value());
      double lon, lat;
      while (iss.good()) {
        iss >> lon;
        iss >> lat;
        AIXM_coordinate m_cord;
        m_cord.m_Lat = lat;
        m_cord.m_Lon = lon;

        // Find min/max coords
        m_min_coord.m_Lat = std::min(m_cord.m_Lat, m_min_coord.m_Lat);
        m_min_coord.m_Lon = std::min(m_cord.m_Lon, m_min_coord.m_Lon);
        m_max_coord.m_Lat = std::max(m_cord.m_Lat, m_max_coord.m_Lat);
        m_max_coord.m_Lon = std::max(m_cord.m_Lon, m_max_coord.m_Lon);
        gml_object.m_Coordinates.push_back(m_cord);
      }

      if (gml_object.m_AIXM_object_type == "GuidanceLine") {
        airport.m_taxiways.emplace_back(gml_object);
      } else if (gml_object.m_AIXM_object_type == "TaxiwayElement") {
        airport.m_taxiways.emplace_back(gml_object);
      } else if (gml_object.m_AIXM_object_type == "RunwayElement") {
        airport.m_runways.push_back(gml_object);
      } else {
        airport.m_gates.push_back(gml_object);
      }
    }
  }

  return true;
}

double distance(double p_lon1, double p_lat1, double p_lon2, double p_lat2) {
  double R = 6371; // km
  double lat1_rad = p_lat1 * (M_PI / 180.0);
  double lat2_rad = p_lat2 * (M_PI / 180.0);
  double dlat = (p_lat2 - p_lat1) * (M_PI / 180.0);
  double dlon = (p_lon2 - p_lon1) * (M_PI / 180.0);
  double a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1_rad) * cos(lat2_rad) * sin(dlon / 2) * sin(dlon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double d = R * c;
  return d;
}

double bearing_rad(double p_lon1, double p_lat1, double p_lon2, double p_lat2) {
  double y = sin(p_lon2 - p_lon1) * cos(p_lat2);
  double x = cos(p_lat1) * sin(p_lat2) - sin(p_lat1) * cos(p_lat2) * cos(p_lon2 - p_lon1);
  return atan2(y, x);
}
