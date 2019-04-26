#ifndef GATE_H
#define GATE_H

#include "GML_object.hpp"
#include <vector>

class gate {
public:
  //! Default constructor
  gate(GMLObject gate_data);
  std::vector<GMLObject> m_vertices;
};

#endif /* GATE_H */
