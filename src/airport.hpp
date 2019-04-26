#ifndef AIRPORT_H
#define AIRPORT_H

#include <vector>

#include "aircraft.hpp"
#include "gate.hpp"
#include "runway.hpp"
#include "taxiway.hpp"

class airport {
public:
  using visitor_t = std::function<void(const std::vector<GMLObject> &)>;

  //! Default constructor
  airport() = default;

  //! Copy constructor
  airport(const airport &other) = delete;

  //! Move constructor
  airport(airport &&other) = delete;

  void visit_coordinates(visitor_t fn) const;
  // protected:
  // private:
  std::vector<runway> m_runways;
  std::vector<taxiway> m_taxiways;
  std::vector<gate> m_gates;
  std::vector<aircraft> m_aircrafts;
};

#endif /* AIRPORT_H */
