#pragma once
#include "P3.h"
#include <array>
#include <memory>
class QuadTree;

class Node {

public:
  Node();
  Node(QuadTree *p_parentTree);
  ~Node(void);
  void init();
  // Node type
  inline bool is_root() const { return !m_parent_node; }
  inline bool is_leaf() const { return !m_child_nodes && m_parent_node; }
  inline bool is_node() const { return m_parent_node && m_child_nodes; };

  // Node ID
  int m_node_id;

  // Node depth
  int m_depth;

  // parent Tree
  QuadTree *m_parent_tree;

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
  std::unique_ptr<std::array<Node, 4>> m_child_nodes;

  // Pointer to parent node
  Node *m_parent_node = nullptr;

  // Node counter
  static unsigned int nodecount;

  // Path routing

  // A* algorithm parent
  Node *A_Parent;

  // A* Cost
  double cost;
  double f_cost;

  // Boolean to indicate when a node need to be redrawn.
  bool m_draw = true;
};
