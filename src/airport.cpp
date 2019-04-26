#include "airport.hpp"

void airport::visit_coordinates(airport::visitor_t fn) const {
  for (auto &runway : m_runways) {
    fn(runway.m_vertices);
  }

  for (auto &taxiway : m_taxiways) {
    fn(taxiway.m_vertices);
  }

  for (auto &gate : m_gates) {
    fn(gate.m_vertices);
  }
}
