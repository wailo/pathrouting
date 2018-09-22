#include "AIXM_file_parser.h"
#include "QuadTree.h"
#include <functional>
#include <math.h>

AIXM_file_parser::AIXM_file_parser(void) {}

AIXM_file_parser::~AIXM_file_parser(void) {}

bool AIXM_file_parser::read_AIXM_file(std::string full_path) {
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(full_path.c_str());

  if (!result) {
    throw std::runtime_error("AIXM file does not exist:" + full_path);
  }

  const auto &aixm_members = doc.select_nodes("//message:hasMember", 0);
  for (const auto &member_itr : aixm_members) {

    const auto &node = member_itr.node();
    if (!strcmp(node.name(), "message:hasMember")) {

      AIXM_file_parser::GMLObject gml_object;
      std::string temp_str = std::string(node.first_child().name());
      std::size_t pos = temp_str.find_first_of(':');
      temp_str = temp_str.substr(pos + 1);
      gml_object.m_AIXM_object_type = temp_str;
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
        Coordinate m_cord;
        m_cord.m_Lat = lat;
        m_cord.m_Lon = lon;

        double temp_distance = distance(0, 0, lon, lat);
        double temp_bearing = bearing_rad(lon, lat, 0, 0);

        m_cord.m_X = temp_distance * cos(temp_bearing);
        m_cord.m_Y = temp_distance * sin(temp_bearing);
        gml_object.m_Coordinates.push_back(m_cord);
      }

      Objects.push_back(gml_object);
    }
  }
  return true;
}

void AIXM_file_parser::process_boundaries(QuadTree &tree) {

  std::for_each(Objects.begin(), Objects.end(), [this](AIXM_file_parser::GMLObject p_poly) {
    const double DEG_TO_RAD = M_PI / 180.0;
    for (int i = 0; i < p_poly.m_Coordinates.size(); ++i) {
      p_poly.m_Coordinates.at(i).m_Y =
          y_distance(p_poly.m_Coordinates.at(i).m_Lon * DEG_TO_RAD, p_poly.m_Coordinates.at(i).m_Lat * DEG_TO_RAD);
      p_poly.m_Coordinates.at(i).m_X =
          x_distance(p_poly.m_Coordinates.at(i).m_Lon * DEG_TO_RAD, p_poly.m_Coordinates.at(i).m_Lat * DEG_TO_RAD);
    }
  });

  Coordinate min_coord, max_coord;

  min_coord.m_Lat = std::numeric_limits<double>::max();
  min_coord.m_Lon = std::numeric_limits<double>::max();
  max_coord.m_Lat = std::numeric_limits<double>::lowest();
  max_coord.m_Lon = std::numeric_limits<double>::lowest();

  for (const auto &obj : Objects) {
    for (const auto &coord : obj.m_Coordinates) {
      min_coord.m_Lat = std::min(coord.m_Lat, min_coord.m_Lat);
      min_coord.m_Lon = std::min(coord.m_Lon, min_coord.m_Lon);
      max_coord.m_Lat = std::max(coord.m_Lat, max_coord.m_Lat);
      max_coord.m_Lon = std::max(coord.m_Lon, max_coord.m_Lon);
    }
  }

  double tol1 = (max_coord.m_Lon - min_coord.m_Lon) * 0.001;
  double tol2 = (max_coord.m_Lat - min_coord.m_Lat) * 0.001;

  tree.updateTreeBoundary(min_coord.m_Lon - tol1, max_coord.m_Lon + tol1, min_coord.m_Lat - tol2,
                          max_coord.m_Lat + tol2);

  int kk = 0;
  for (auto &p_poly : Objects) {
    kk++;
    for (auto i = 1; i < p_poly.m_Coordinates.size(); ++i) {
      Node *node = tree.findTreeNode(p_poly.m_Coordinates.at(i).m_Lon, p_poly.m_Coordinates.at(i).m_Lat);
      Node *node1 = tree.findTreeNode(p_poly.m_Coordinates.at(i - 1).m_Lon, p_poly.m_Coordinates.at(i - 1).m_Lat);
      while (node == node1 /*false*/) {
        node1 = tree.findTreeNode(p_poly.m_Coordinates.at(i).m_Lon, p_poly.m_Coordinates.at(i).m_Lat, node1);
        tree.constructTreeNode(node1);
      }
    }
  }
}

float AIXM_file_parser::mapDistance(float dLat, float dLon) {

  float toRad = (M_PI / 180.00);
  float R = 6371; // km
  dLat *= toRad;
  dLon *= toRad;
  float p_lat1 = 0;
  float p_lat2 = 0;

  float a = sin(dLat / 2) * sin(dLat / 2) + (cos(p_lat1) * cos(p_lat2) * sin(dLon / 2) * sin(dLon / 2));

  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  float d = R * c;
  return d;
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

double AIXM_file_parser::x_distance(double p_lon1, double p_lat1) { return 6371.0 * sin(p_lat1) * cos(p_lon1); }

double AIXM_file_parser::y_distance(double p_lon1, double p_lat1) { return 6371.0 * sin(p_lat1) * sin(p_lon1); }

double AIXM_file_parser::polarTodec(std::string polarCoord) {
  std::error_code format_error;
  std::regex Latitude(R"(^\-?(\\d|[0-8][0-9]|90)+\\s\\d\\d\\s\\d\\d+[NSWE])");

  if (!regex_match(polarCoord.begin(), polarCoord.end(), Latitude)) {
    throw format_error;
  }

  if (polarCoord.back() == 'S' || polarCoord.back() == 'W') {
    polarCoord.insert(polarCoord.begin(), '-');
  }

  // Remove the last char
  polarCoord.pop_back();
  double dlat = 0;
  int token = 0;
  std::stringstream stream1;
  stream1.str(polarCoord);

  for (auto i = 0; i < 3; ++i) {
    stream1 >> token;
    dlat += token / pow(60.00, i);
  }

  return dlat;
}
