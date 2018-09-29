#include "QuadTree.h"
#include "GLWidget.h"
#include <QtDebug>
#include <algorithm>

int QuadTree::maxGridDepth = 1;

QuadTree::QuadTree(int LT, int RT, int BT, int TP)
    : right(RT), left(LT), bottom(BT), top(TP), gridWidth(0.0), gridHeight(0.0), m_rootNode(this) {

  // NW, SW, NE, SE
  h_order[0] = 0;
  h_order[1] = 0;
  h_order[2] = 1;
  h_order[3] = 1;

  v_order[0] = 0;
  v_order[1] = 1;
  v_order[2] = 0;
  v_order[3] = 1;

  // initialize the rootNode here
  gridWidth = fabs(right - left);
  gridHeight = fabs(top - bottom);

  InitRootNode();
}

QuadTree::~QuadTree(void) { removeTreeNode(m_rootNode); }

void QuadTree::InitRootNode() {
  // Start node id from 1
  m_rootNode.m_node_id = 1;
  // Start depth from 1
  m_rootNode.m_depth = 1;

  // set root node parent to NULL
  m_rootNode.m_parent_node = nullptr;
}

double QuadTree::get_right() { return right; }
double QuadTree::get_left() { return left; }
double QuadTree::get_bottom() { return bottom; }
double QuadTree::get_top() { return top; }

void QuadTree::removeTreeNode(Node &pNode) {

  // If this node is not the root node, then assign its type to leaf node
  if (pNode.m_parent_node) {
    if (!pNode.m_parent_node->is_root()) {
    }
  }
}

// Retrieve Tree node that contains a point defined by XY
Node *QuadTree::findTreeNode(double x, double y, Node *p_startNode) {

  if (!p_startNode) {
    p_startNode = &m_rootNode;
  }

  // Check first if the point is inside the boundaries!!
  if (!((x <= p_startNode->centre_x() + p_startNode->x_dsp()) &&
        (x >= p_startNode->centre_x() - p_startNode->x_dsp()) &&
        (y <= p_startNode->centre_y() + p_startNode->y_dsp()) &&
        (y >= p_startNode->centre_y() - p_startNode->y_dsp()))) {
    // If outside the boundary
    throw std::runtime_error("X Y coords is outside the tree boundary and will never be found");
  }

  Node *pNode;
  unsigned char bit = 0;

  pNode = p_startNode;
  while (!pNode->is_leaf()) {
    // reset the bit to [0|0]
    bit = 0;

    // bit structure
    // [0|0]
    // [x|y]
    if (y >= pNode->centre_y()) {
      // set the first bit to One
      bit |= 1;
    }

    else {
      // set the first bit to Zero
      bit |= 0;
    }

    if (x >= pNode->centre_x()) {
      // set the second bit to One
      bit |= 2;
    }

    else {
      // set the second bit to Zero
      bit |= 0;
    }

    // This is for a special case where the root node is initilized but not expanded yet
    // check if the node is valid
    if (pNode->m_child_nodes) {
      pNode = &pNode->m_child_nodes.get()->at(bit);
    } else {
      break;
    }
  }

  return pNode;
}

// Construct a tree branch by constructing all the children nodes
void QuadTree::constructTreeNode(Node *node) {
  // If node is not a leaf then this will cause memory leak!
  if (node->is_node()) {
    throw std::runtime_error("Node is not a leaf node. Potential memory leak");
  }

  assert(node->m_child_nodes == nullptr);
  node->m_child_nodes.reset(new std::array<Node, 4>());

  // problem here, this function create duplicate nodes!
  for (unsigned int i = 0; i < 4; ++i) {
    // Initialize the node
    node->m_child_nodes.get()->at(i).m_parent_tree = this;
    node->m_child_nodes.get()->at(i).m_parent_node = node;
    node->m_child_nodes.get()->at(i).m_depth = node->m_depth + 1;
    node->m_child_nodes.get()->at(i).m_node_id = (4 * node->m_node_id) - 2 + i;

    maxGridDepth = std::max(node->m_child_nodes.get()->at(i).m_depth, maxGridDepth);
  }
}

void QuadTree::updateTreeBoundary(double LT, double RT, double BT, double TP) {
  left = LT;
  right = RT;
  bottom = BT;
  top = TP;

  gridWidth = fabs(right - left);
  gridHeight = fabs(top - bottom);
}

void QuadTree::forEachNode(Node *pRootnode, OnDrawEventHandler func) {
  // Function to be applied at each node, the function should modify the target node only!
  if (!pRootnode) {
    return;
  }

  func(pRootnode);

  if (!pRootnode->is_leaf()) {
    for (unsigned int i = 0; i < 4; ++i) {
      // This is for a special case where the root node is initilized but not expanded yet
      // check if the node is valid
      if (pRootnode->m_child_nodes) {
        forEachNode(&pRootnode->m_child_nodes.get()->at(i), func);
      }
    }
  }
}

int QuadTree::getNodeCount() const { return Node::nodecount; }

// Function to balance the tree by enforcing [2:1] rule
void QuadTree::balanceTree(Node *P) {
  Node *tempPtr;

  for (int i = NW; i <= W; ++i) {
    tempPtr = findNeighbour(P, i);

    if ((P->m_depth - tempPtr->m_depth) > 1 && (tempPtr->is_leaf())) {

      constructTreeNode(tempPtr);
      for (unsigned int j = 0; j < 4; ++j) {
        balanceTree(&tempPtr->m_child_nodes.get()->at(j));
      }
    }
  }
}

void recursive(Node *pNode, int D1, int D2, std::vector<Node *> &vector) {

  if (pNode->is_leaf()) {
    vector.push_back(pNode);
  }

  else {
    recursive(&pNode->m_child_nodes.get()->at(D1), D1, D2, vector);
    recursive(&pNode->m_child_nodes.get()->at(D2), D1, D2, vector);
  }
}

void QuadTree::getAllNeighbours(Node *thisNode, std::vector<Node *> &vector) {
  // Iterate through all directions
  for (int Direction = NW; Direction <= W; ++Direction) {
    // Copy node
    Node *pNeighbourNode;

    switch (Direction) {
      // Eastern
    case E:

      // Get Eastern node
      pNeighbourNode = findTreeNode(
          thisNode->centre_x() + (thisNode->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
          thisNode->centre_y());

      // If the neighbouring node is not this node (flaw in neighbour finding function)
      if (pNeighbourNode->m_node_id != thisNode->m_node_id) {
        // traverse through the neighbouring node parents until the parent with the same depth is found
        while (pNeighbourNode->m_depth > thisNode->m_depth) {
          pNeighbourNode = pNeighbourNode->m_parent_node;
        }

        // Now find all the NW and SW children of the neighbouring node that share the same edge with this node and add
        // the to a container
        recursive(pNeighbourNode, NW, SW, vector);
      }

      break;

    case W:
      pNeighbourNode = findTreeNode(
          thisNode->centre_x() - (thisNode->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
          thisNode->centre_y());

      if (pNeighbourNode->m_node_id != thisNode->m_node_id) {
        while (pNeighbourNode->m_depth > thisNode->m_depth) {
          pNeighbourNode = pNeighbourNode->m_parent_node;
        }
        recursive(pNeighbourNode, NE, SE, vector);
      }

      break;

    case N:
      pNeighbourNode =
          findTreeNode(thisNode->centre_x(),
                       thisNode->centre_y() -
                           (thisNode->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)) + 0.005);

      if (pNeighbourNode->m_node_id != thisNode->m_node_id) {
        while (pNeighbourNode->m_depth > thisNode->m_depth) {
          pNeighbourNode = pNeighbourNode->m_parent_node;
        }
        recursive(pNeighbourNode, SW, SE, vector);
      }

      break;

    case S:
      pNeighbourNode = findTreeNode(
          thisNode->centre_x(),
          thisNode->centre_y() + (thisNode->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
      if (pNeighbourNode->m_node_id != thisNode->m_node_id) {
        while (pNeighbourNode->m_depth > thisNode->m_depth) {
          pNeighbourNode = pNeighbourNode->m_parent_node;
        }
        recursive(pNeighbourNode, NW, NE, vector);
      }
      break;

    case NW:
      pNeighbourNode = findTreeNode(
          thisNode->centre_x() - (thisNode->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
          thisNode->centre_y() - (thisNode->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
      if (pNeighbourNode->m_node_id != thisNode->m_node_id) {
        vector.push_back(pNeighbourNode);
      }
      break;

    case NE:
      pNeighbourNode = findTreeNode(
          thisNode->centre_x() + (thisNode->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
          thisNode->centre_y() - (thisNode->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
      if (pNeighbourNode->m_node_id != thisNode->m_node_id) {
        vector.push_back(pNeighbourNode);
      }
      break;

    case SW:
      pNeighbourNode = findTreeNode(
          thisNode->centre_x() - (thisNode->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
          thisNode->centre_y() + (thisNode->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
      if (pNeighbourNode->m_node_id != thisNode->m_node_id) {
        vector.push_back(pNeighbourNode);
      }
      break;

    case SE:
      pNeighbourNode = findTreeNode(
          thisNode->centre_x() + (thisNode->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
          thisNode->centre_y() + (thisNode->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
      if (pNeighbourNode->m_node_id != thisNode->m_node_id) {
        vector.push_back(pNeighbourNode);
      }
      break;
    }
  }

  // Remove duplicates from the vector

  // Predicates lambda functions
  auto compare_nodes = [](Node *i, Node *j) -> bool { return i->m_node_id == j->m_node_id; };
  auto sort_nodes = [](Node *i, Node *j) -> bool { return i->m_node_id < j->m_node_id; };

  // Sort first
  std::sort(vector.begin(), vector.end(), sort_nodes);

  // then remove duplicates
  vector.erase(std::unique(vector.begin(), vector.end(), compare_nodes), vector.end());
}

// Find Neighbouring node using geometric coordinates, return leaf node
Node *QuadTree::findLeafNeighbour(Node *p, int Direction) {
  switch (Direction) {
  case E:
    return findTreeNode(p->centre_x() + (p->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
                        p->centre_y());
    break;

  case W:
    return findTreeNode(p->centre_x() - (p->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
                        p->centre_y());
    break;

  case N:

    return findTreeNode(p->centre_x(),
                        p->centre_y() - (p->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
    break;

  case S:
    return findTreeNode(p->centre_x(),
                        p->centre_y() + (p->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
    break;

  case NW:
    return findTreeNode(p->centre_x() - (p->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
                        p->centre_y() - (p->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
    break;

  case NE:
    return findTreeNode(p->centre_x() + (p->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
                        p->centre_y() - (p->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
    break;

  case SW:
    return findTreeNode(p->centre_x() - (p->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
                        p->centre_y() + (p->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
    break;

  case SE:
    return findTreeNode(p->centre_x() + (p->x_dsp() + (Node::x_dsp(get_left(), get_right(), maxGridDepth) / 2.0)),
                        p->centre_y() + (p->y_dsp() + (Node::y_dsp(get_top(), get_bottom(), maxGridDepth) / 2.0)));
    break;

  default:
    throw std::runtime_error("Unknown Direction");
  };
}

// Find Neighbours using pointer relationship, FAST operation, but return node at the same depth, ignoring child nodes
Node *QuadTree::findNeighbour(Node *p, int Direction) {
  if (p == &this->m_rootNode) {
    // no neighbour for the parent node
    return &this->m_rootNode;
  }

  // NW, SW, NE, SE
  switch (Direction) {
  case E:

    // if p = NW-child
    if (p == &p->m_parent_node->m_child_nodes.get()->at(NW)) {
      // then return the NE
      return &p->m_parent_node->m_child_nodes.get()->at(NE);
    }

    // if p = SW-child
    else if (p == &p->m_parent_node->m_child_nodes.get()->at(SW)) {
      // then return the SE
      return &p->m_parent_node->m_child_nodes.get()->at(SE);
    }
    break;

  case W:

    // if p = NW-child
    if (p == &p->m_parent_node->m_child_nodes.get()->at(NE)) {
      // then return the NE
      return &p->m_parent_node->m_child_nodes.get()->at(NW);
    }

    // if p = SW-child
    else if (p == &p->m_parent_node->m_child_nodes.get()->at(SE)) {
      // then return the SE
      return &p->m_parent_node->m_child_nodes.get()->at(SW);
    }
    break;

  case N:

    // if p = NW-child
    if (p == &p->m_parent_node->m_child_nodes.get()->at(SE)) {
      // then return the NE
      return &p->m_parent_node->m_child_nodes.get()->at(NE);
    }

    // if p = SW-child
    else if (p == &p->m_parent_node->m_child_nodes.get()->at(SW)) {
      // then return the SE
      return &p->m_parent_node->m_child_nodes.get()->at(NW);
    }
    break;

  case S:

    // if p = NW-child
    if (p == &p->m_parent_node->m_child_nodes.get()->at(NE)) {
      // then return the NE
      return &p->m_parent_node->m_child_nodes.get()->at(SE);
    }

    // if p = SW-child
    else if (p == &p->m_parent_node->m_child_nodes.get()->at(NW)) {
      // then return the SE
      return &p->m_parent_node->m_child_nodes.get()->at(SW);
    }
    break;

  case SW:
    return findNeighbour(findNeighbour(p, S), W);
    break;

  case NW:
    return findNeighbour(findNeighbour(p, N), W);
    break;

  case SE:
    return findNeighbour(findNeighbour(p, S), E);
    break;

  case NE:
    return findNeighbour(findNeighbour(p, N), E);
    break;
  }

  // another swictch statement here

  // return the east neighbour of the parent cell
  Node *p1 = findNeighbour(p->m_parent_node, Direction);

  if (p1->is_leaf() || p1->is_root()) {
    return p1;
  }

  else {
    switch (Direction) {
    case E:
      // if p == NE-Child
      if (p == &p->m_parent_node->m_child_nodes.get()->at(NE)) {
        // return the NW-Child of p1
        return &p1->m_child_nodes.get()->at(NW);
      }

      else {
        // return the SW-Child of p1
        return &p1->m_child_nodes.get()->at(SW);
      }
      break;

    case W:
      // if p == NE-Child
      if (p == &p->m_parent_node->m_child_nodes.get()->at(NW)) {
        // return the NW-Child of p1
        return &p1->m_child_nodes.get()->at(NE);
      }

      else {
        // return the SW-Child of p1
        return &p1->m_child_nodes.get()->at(SE);
      }
      break;

    case N:
      // if p == NE-Child
      if (p == &p->m_parent_node->m_child_nodes.get()->at(NW)) {
        // return the NW-Child of p1
        return &p1->m_child_nodes.get()->at(SW);
      }

      else {
        // return the SW-Child of p1
        return &p1->m_child_nodes.get()->at(SE);
      }
      break;

    case S:
      // if p == NE-Child
      if (p == &p->m_parent_node->m_child_nodes.get()->at(SW)) {
        // return the NW-Child of p1
        return &p1->m_child_nodes.get()->at(NW);
      }

      else {
        // return the SW-Child of p1
        return &p1->m_child_nodes.get()->at(NE);
      }
      break;
    }
  }
  throw std::runtime_error("Could not find a neighbour");
}

Node *QuadTree::findNeighbour(Node *p, char Direction) {
  boost::dynamic_bitset<> xbit;
  boost::dynamic_bitset<> ybit;

  while (!p->is_root()) {
    qDebug() << p->centre_x() << " - x - " << p->m_parent_node->centre_x() << endl;
    qDebug() << p->centre_y() << " - y - " << p->m_parent_node->centre_y() << endl;

    if (p->centre_x() > p->m_parent_node->centre_x()) {
      xbit.push_back(1);
    }

    else {
      xbit.push_back(0);
    }

    if (p->centre_y() > p->m_parent_node->centre_y()) {
      ybit.push_back(1);
    }

    else {
      ybit.push_back(0);
    }

    p = p->m_parent_node;
  }
  if (xbit.count() == xbit.size()) {
    qDebug() << " edge node ";
    // returning the root node for testing only. should return the input node.
    return p;
  }
  // adjust the binary as per the requested direction
  xbit = boost::dynamic_bitset<>(maxGridDepth, xbit.to_ulong() + 1);
  ybit = boost::dynamic_bitset<>(maxGridDepth, ybit.to_ulong());

  // convery binary to coordinates
  double xCor = 0;
  double yCor = 0;

  for (int i = 0; i < xbit.size(); i++) {
    qDebug().maybeSpace() << xbit[i];
  }
  qDebug() << " end of bit array";

  for (int i = xbit.size() - 1; i >= 0; --i) {
    xCor += xbit[i] * (gridWidth / (pow(2.00, int(i + 1))));
    qDebug() << xbit[i] << endl;
    yCor += ybit[i] * (gridHeight / (pow(2.00, int(i + 1))));
  }

  qDebug() << xCor << " - yCor - " << yCor << endl;

  return findTreeNode(xCor, yCor);
}

double QuadTree::distance(Node *start, Node *finish) {
  P3 temp_p3;
  temp_p3.x = start->centre_x() - finish->centre_x();
  temp_p3.y = start->centre_y() - finish->centre_y();
  // Calculate the distance
  return sqrt((temp_p3.x * temp_p3.x) + (temp_p3.y * temp_p3.y));
}

Node *getNodeWithLowestCost(std::map<int, Node *> &map) {

  auto sort_by_cost = [](std::pair<int, Node *> const i, std::pair<int, Node *> const j) {
    return (i.second->f_cost) < (j.second->f_cost);
  };

  return std::min_element(map.begin(), map.end(), sort_by_cost)->second;
}

bool QuadTree::path_routing(Node *start, Node *finish) {

  Node *current;
  closedSet.clear();
  camefromSet.clear();
  openedSet.clear();

  openedSet[start->m_node_id] = start;
  start->cost = 0;
  start->f_cost = 0;

  while (!openedSet.empty()) {
    current = getNodeWithLowestCost(openedSet);

    qDebug() << "Node with lowest cost is: " << current->m_node_id;
    // Node Found!
    if (current == finish) {
      qDebug() << "Path found";

      Node *tnode = finish;
      camefromSet.clear();
      camefromSet.push_back(tnode);

      while (tnode->m_node_id != start->m_node_id) {
        camefromSet.push_back(tnode);
        tnode = tnode->A_Parent;
      }

      camefromSet.push_back(start);
      return true;
    }

    else {
      openedSet.erase(current->m_node_id);
      closedSet[current->m_node_id] = current;

      // for each neighbour

      getAllNeighbours(current, neighbours);

      // Initialize for node scanning
      double tentative_cost = 0;
      auto neighbour_itr = neighbours.begin();
      while (neighbour_itr != neighbours.end()) {
        tentative_cost = current->cost + distance(current, (*neighbour_itr));

        qDebug() << "tentative cost: " << tentative_cost << " nbr_node: " << (*neighbour_itr)->m_node_id;
        // if the node in the closed list
        if (closedSet.find((*neighbour_itr)->m_node_id) != closedSet.end() /*&& neighbour_itr->depth != 4*/) {
          if (tentative_cost >= (*neighbour_itr)->cost) {
            ++neighbour_itr;
            continue;
          }
        }

        // if the node not in the opened list
        if (openedSet.find((*neighbour_itr)->m_node_id) == openedSet.end() || tentative_cost < (*neighbour_itr)->cost) {
          // add more statements (node not walkable)
          (*neighbour_itr)->A_Parent = current;
          (*neighbour_itr)->cost = tentative_cost;
          (*neighbour_itr)->f_cost = tentative_cost + distance((*neighbour_itr), finish);
        }

        // if the node not in the closed list
        if (closedSet.find((*neighbour_itr)->m_node_id) == closedSet.end() /*&& neighbour_itr->depth != 4*/) {
          // add neighbour to openset
          openedSet[(*neighbour_itr)->m_node_id] = (*neighbour_itr);
        }

        // neighbours.pop_back();
        ++neighbour_itr;
      }
      // Not needed anymore, clear the container
      neighbours.clear();
    }
  }

  // Return failure;
  qDebug() << "No path found!";
  return false;
}
