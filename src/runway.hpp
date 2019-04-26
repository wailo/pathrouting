#ifndef RUNWAY_H
#define RUNWAY_H

#include "GML_object.hpp"
#include <vector>

class runway {
public:
  runway(GMLObject runway_data);
  std::vector<GMLObject> m_vertices;
};

#endif /* RUNWAY_H */
