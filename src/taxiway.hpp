#ifndef TAXIWAY_H
#define TAXIWAY_H

#include "GML_object.hpp"
#include <vector>

class taxiway {
public:
  taxiway(GMLObject taxiway_data);
  std::vector<GMLObject> m_vertices;
};

#endif /* TAXIWAY_H */
