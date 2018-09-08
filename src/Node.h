#pragma once
#include "P3.h"

class QuadTree;

class Node {

public:
  enum nodeType { ROOT_TYPE, NODE_TYPE, LEAF_TYPE };

  Node(QuadTree *p_parentTree);
  ~Node(void);

  // Node type
  nodeType type;

  // Node ID
  int id;

  // Node depth
  short depth;

  // parent Tree
  QuadTree *m_parentTree;

  // Calculate this node displacement value, which is the distance from centre point to the edge of node, same as raduis
  double x_dsp();
  double y_dsp();

  // Calculate node displacement value at given depth. Displacement is the distance from centre point to the edge of
  // node, same as raduis
  double x_dsp(int depth);
  double y_dsp(int depth);

  double centre_x();
  double centre_y();
  double centre_z();

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
