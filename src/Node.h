#pragma once
#include "P3.h"

class QuadTree;

class Node {

public:
  Node(QuadTree *p_parentTree);
  ~Node(void);

  // Node type
  inline bool is_leaf() const { return !Child[0]; }
  inline bool is_root() const { return !Parent; }
  inline bool is_node() const { return Parent && Child[0]; };

  // Node ID
  int id;

  // Node depth
  int depth;

  // parent Tree
  QuadTree *m_parentTree;

  // Calculate this node displacement value, which is the distance from centre point to the edge of node, same as raduis
  double x_dsp() const;
  double y_dsp() const;

  // Calculate node displacement value at given depth. Displacement is the distance from centre point to the edge of
  // node, same as raduis
  double x_dsp(int depth) const;
  double y_dsp(int depth) const;

  double centre_x() const;
  double centre_y() const;
  double centre_z() const;

  // Pointer to childern nodes
  Node *Child[4];

  // Pointer to parent node
  Node *Parent;

  // Node counter
  static unsigned int nodecount;

  // Path routing

  // A* algorithm parent
  Node *A_Parent;

  // A* Cost
  double cost;
  double f_cost;

  // Boolean to indicate when a node need to be redrawn.
  bool m_draw;
};
