#pragma once
#include "P3.h"
#include <array>
#include <memory>
class QuadTree;

class Node {

public:
  // alias type for child nodes
  using child_nodes_t = std::array<std::unique_ptr<Node>, 4>;

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
  // Root node does not have a parent
  inline bool is_root() const { return !m_parent_node; }
  // leaf node, does have a parent, no child nodes
  inline bool is_leaf() const {
    return std::all_of(m_child_nodes.begin(), m_child_nodes.end(),
                       [](std::unique_ptr<Node> const &p) { return p == nullptr; }) &&
           m_parent_node;
  }
  // Normal node does have a parent and child nodes
  inline bool is_node() const {
    return !std::all_of(m_child_nodes.begin(), m_child_nodes.end(), [](std::unique_ptr<Node> const &p) {
      return p == nullptr;
    }) && m_parent_node;
  };

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

  // Child nodes
  child_nodes_t m_child_nodes;

  // Node ID
  int m_node_id;

  // Node depth
  int m_depth;

  // Pointer to parent node
  Node *m_parent_node = nullptr;

  // parent Tree
  QuadTree *m_parent_tree;
};
