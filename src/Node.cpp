#include "Node.h"
#include "QuadTree.h"
#include <limits>
// Initiaize static member
unsigned int Node::nodecount = 0;

double Node::x_dsp(double left, double right, int depth_) { return fabs(left - right) / pow(2.00, depth_); }

double Node::y_dsp(double top, double bottom, int depth_) { return fabs(top - bottom) / pow(2.00, depth_); }

Node::Node() { init(); }

Node::Node(QuadTree *p_parentTree) : m_parent_tree(p_parentTree) { init(); }

Node::~Node(void) {
  // Node count decremented
  nodecount--;

  // type = NULL;
  m_node_id = 0;
}

void Node::init() {

  // Increment the node counter
  nodecount++;

  // Set the ID to the node counter
  m_node_id = nodecount;

  // For Path routing
  A_Parent = nullptr;
  cost = std::numeric_limits<double>::infinity();
  f_cost = std::numeric_limits<double>::infinity();
}

double Node::x_dsp() const { return fabs(m_parent_tree->get_left() - m_parent_tree->get_right()) / pow(2.00, m_depth); }

double Node::y_dsp() const { return fabs(m_parent_tree->get_top() - m_parent_tree->get_bottom()) / pow(2.00, m_depth); }

double Node::centre_x() const {
  if (is_root()) {
    // in a vector
    return m_parent_tree->left + m_parent_tree->gridWidth - x_dsp();
  }

  else {
    if (this == &m_parent_node->m_child_nodes.get()->at(0)) {
      // Locate the centre point of the node;
      return m_parent_node->centre_x() - (x_dsp()) + (2 * m_parent_tree->h_order[0] * -x_dsp());

    }

    else if (this == &m_parent_node->m_child_nodes.get()->at(1)) {
      // Locate the centre point of the node;
      return m_parent_node->centre_x() - (x_dsp()) + (2 * m_parent_tree->h_order[1] * x_dsp());

    }

    else if (this == &m_parent_node->m_child_nodes.get()->at(2)) {
      // Locate the centre point of the node;
      return m_parent_node->centre_x() - (x_dsp()) + (2 * m_parent_tree->h_order[2] * x_dsp());

    }

    else if (this == &m_parent_node->m_child_nodes.get()->at(3)) {
      // Locate the centre point of the node;
      return m_parent_node->centre_x() - (x_dsp()) + (2 * m_parent_tree->h_order[3] * x_dsp());
    } else {
      // Somthing wrong here!
      throw std::runtime_error("Uknown Node");
      return 0;
    }
  }
}

double Node::centre_y() const {
  if (is_root()) {
    // in a vector
    return m_parent_tree->bottom + m_parent_tree->gridHeight - y_dsp();
  }

  else {
    if (this == &m_parent_node->m_child_nodes.get()->at(0)) {
      // Locate the centre point of the node;
      return m_parent_node->centre_y() - (y_dsp()) + (2 * m_parent_tree->v_order[0] * y_dsp());
    }

    else if (this == &m_parent_node->m_child_nodes.get()->at(1)) {
      // Locate the centre point of the node;
      return m_parent_node->centre_y() - (y_dsp()) + (2 * m_parent_tree->v_order[1] * y_dsp());
    }

    else if (this == &m_parent_node->m_child_nodes.get()->at(2)) {
      // Locate the centre point of the node;
      return m_parent_node->centre_y() - (y_dsp()) + (2 * m_parent_tree->v_order[2] * y_dsp());
    }

    else if (this == &m_parent_node->m_child_nodes.get()->at(3)) {
      // Locate the centre point of the node;
      return m_parent_node->centre_y() - (y_dsp()) + (2 * m_parent_tree->v_order[3] * y_dsp());
    } else {
      // Somthing wrong here!
      throw std::runtime_error("Uknown Node");
      return 0;
    }
  }
}

double Node::centre_z() const { return 0; }
