#include "Node.h"
#include "QuadTree.h"
#include <limits>
// Initiaize static member
unsigned int Node::nodecount = 0;

Node::Node() { init(); }

Node::Node(QuadTree *p_parentTree) : m_parentTree(p_parentTree) { init(); }

Node::~Node(void) {
  // Node count decremented
  nodecount--;

  // type = NULL;
  id = 0;
}

void Node::init() {

  // Increment the node counter
  nodecount++;

  // Set the ID to the node counter
  id = nodecount;

  // For Path routing
  A_Parent = nullptr;
  cost = std::numeric_limits<double>::infinity();
  f_cost = std::numeric_limits<double>::infinity();
}

double Node::x_dsp() const { return fabs(m_parentTree->get_left() - m_parentTree->get_right()) / pow(2.00, depth); }

double Node::y_dsp() const { return fabs(m_parentTree->get_top() - m_parentTree->get_bottom()) / pow(2.00, depth); }

double Node::x_dsp(int depth_) const {
  return fabs(m_parentTree->get_left() - m_parentTree->get_right()) / pow(2.00, depth_);
}

double Node::y_dsp(int depth_) const {
  return fabs(m_parentTree->get_top() - m_parentTree->get_bottom()) / pow(2.00, depth_);
}

double Node::centre_x() const {
  if (is_root()) {
    // in a vector
    return m_parentTree->left + m_parentTree->gridWidth - x_dsp();
  }

  else {
    if (this == &Parent->Child.get()->at(0)) {
      // Locate the centre point of the node;
      return Parent->centre_x() - (x_dsp()) + (2 * m_parentTree->h_order[0] * -x_dsp());

    }

    else if (this == &Parent->Child.get()->at(1)) {
      // Locate the centre point of the node;
      return Parent->centre_x() - (x_dsp()) + (2 * m_parentTree->h_order[1] * x_dsp());

    }

    else if (this == &Parent->Child.get()->at(2)) {
      // Locate the centre point of the node;
      return Parent->centre_x() - (x_dsp()) + (2 * m_parentTree->h_order[2] * x_dsp());

    }

    else if (this == &Parent->Child.get()->at(3)) {
      // Locate the centre point of the node;
      return Parent->centre_x() - (x_dsp()) + (2 * m_parentTree->h_order[3] * x_dsp());
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
    return m_parentTree->bottom + m_parentTree->gridHeight - y_dsp();
  }

  else {
    if (this == &Parent->Child.get()->at(0)) {
      // Locate the centre point of the node;
      return Parent->centre_y() - (y_dsp()) + (2 * m_parentTree->v_order[0] * y_dsp());
    }

    else if (this == &Parent->Child.get()->at(1)) {
      // Locate the centre point of the node;
      return Parent->centre_y() - (y_dsp()) + (2 * m_parentTree->v_order[1] * y_dsp());
    }

    else if (this == &Parent->Child.get()->at(2)) {
      // Locate the centre point of the node;
      return Parent->centre_y() - (y_dsp()) + (2 * m_parentTree->v_order[2] * y_dsp());
    }

    else if (this == &Parent->Child.get()->at(3)) {
      // Locate the centre point of the node;
      return Parent->centre_y() - (y_dsp()) + (2 * m_parentTree->v_order[3] * y_dsp());
    } else {
      // Somthing wrong here!
      throw std::runtime_error("Uknown Node");
      return 0;
    }
  }
}

double Node::centre_z() const { return 0; }
