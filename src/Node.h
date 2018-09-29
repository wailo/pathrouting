#pragma once
#include "P3.h"
#include <array>
#include <memory>
class QuadTree;

class Node {

public:
  // Node counter
  static unsigned int nodecount;

  // Calculate node displacement value at given depth. Displacement is the distance from centre point to the edge of
  // node, same as raduis
  static double x_dsp(double left, double right, int depth_);
  static double y_dsp(double top, double bottom, int depth_);

  Node();
  Node(QuadTree *p_parentTree);
  ~Node(void);
  void init();
  // Node type
  inline bool is_root() const { return !m_parent_node; }
  inline bool is_leaf() const { return !m_child_nodes && m_parent_node; }
  inline bool is_node() const { return m_parent_node && m_child_nodes; };

  // Calculate this node displacement value, which is the distance from centre point to the edge of node, same as raduis
  double x_dsp() const;
  double y_dsp() const;

  double centre_x() const;
  double centre_y() const;
  double centre_z() const;

  // Path routing
  // A* algorithm parent
  Node *A_Parent;

  // A* Cost
  double cost;
  double f_cost;

  // Pointer to childern nodes
  std::unique_ptr<std::array<Node, 4>> m_child_nodes;

  // Node ID
  int m_node_id;

  // Node depth
  int m_depth;

  // Pointer to parent node
  Node *m_parent_node = nullptr;

  // parent Tree
  QuadTree *m_parent_tree;
};
