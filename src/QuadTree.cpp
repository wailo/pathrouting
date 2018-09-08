#include "QuadTree.h"
#include "GLWidget.h"
#include <QtDebug>
#include <algorithm>

double QuadTree::gridWidth = 0;
double QuadTree::gridHeight = 0;

int QuadTree::maxGridDepth = 1;

QuadTree::QuadTree(int LT, int RT, int BT, int TP) : right(RT), left(LT), bottom(BT), top(TP), m_rootNode(NULL) {

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

QuadTree::~QuadTree(void) {
  forEachNode(m_rootNode, [&](Node *p_node) { removeTreeNode(p_node); });
}

void QuadTree::InitRootNode() {
  if (!m_rootNode) {
    m_rootNode = new Node(this);
  }

  // Start node id from 1
  m_rootNode->id = 1;
  // Start depth from 1
  m_rootNode->depth = 1;

  // set root node parent to NULL
  m_rootNode->Parent = NULL;

  // Set node type
  m_rootNode->type = Node::ROOT_TYPE;
}

double QuadTree::get_right() { return right; }
double QuadTree::get_left() { return left; }
double QuadTree::get_bottom() { return bottom; }
double QuadTree::get_top() { return top; }

void QuadTree::invalidate_draw() {
   invalidate_draw(m_rootNode);
}


void QuadTree::invalidate_draw(Node* node) {

  if (!node) {
    return;
  }
                
  if (!node->m_draw) {
    node->m_draw = true;
    for (auto child : node->Child) {
      invalidate_draw(child);
    }
  }
}

void QuadTree::removeTreeNode(Node *(&pNode)) {
  // pass the parent node to delete it and its children
  if (pNode != NULL) {
    // IF the node has children
    if (pNode->type != Node::LEAF_TYPE) {
      for (unsigned int i = 0; i < 4; ++i) {
        // Remove the all the child nodes recursively
        removeTreeNode(pNode->Child[i]);
      }

      // qDebug() << " Node removed id" << pNode->id<< " count " << Node::nodecount;

      // If this node is not the root node, then assign its type to leaf node
      if (pNode->Parent) {
        if (pNode->Parent->type != Node::ROOT_TYPE) {
          pNode->Parent->type = Node::LEAF_TYPE;
          pNode->Parent->m_draw = true;
        }
      }
    }

    // free the memory
    delete pNode;

    // Set the pointer to NULL
    pNode = NULL;
  }
}

// Retrieve Tree node that contains a point defined by XY
Node *QuadTree::findTreeNode(double x, double y, Node *p_startNode) {

  // If the rootNode is not initialized
  if (!m_rootNode) {
    InitRootNode();
  }

  if (!p_startNode) {
    p_startNode = m_rootNode;
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
  while (pNode->type != Node::LEAF_TYPE) {
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
    if (pNode->Child[bit]) {
      pNode = pNode->Child[bit];
    } else {
      break;
    }
  }

  return pNode;
}

// Construct a tree branch by constructing all the children nodes
void QuadTree::constructTreeNode(Node *node) {
  // If node is not a leaf then this will cause memory leak!
  if (node->type == Node::NODE_TYPE) {
    throw std::runtime_error("Node is not a leaf node. Potential memory leak");
  }

  // problem here, this function create duplicate nodes!
  for (unsigned int i = 0; i < 4; ++i) {
    if (node->Child[i] == NULL) // To prevent creating duplicate nodes
    {
      // Initialize the node
      node->Child[i] = new Node(this);

      node->Child[i]->Parent = node;
      node->Child[i]->depth = node->Child[i]->Parent->depth + 1;
      if (node->Child[i]->depth > maxGridDepth) {
        maxGridDepth = node->Child[i]->depth;
      }
      node->Child[i]->type = Node::LEAF_TYPE;
      node->Child[i]->id = (4 * node->Child[i]->Parent->id) - 2 + i;
      if (node->type != Node::ROOT_TYPE) {
        node->type = Node::NODE_TYPE;
      }
    }

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

  if (pRootnode->type != Node::LEAF_TYPE) {
    for (unsigned int i = 0; i < 4; ++i) {
      // This is for a special case where the root node is initilized but not expanded yet
      // check if the node is valid
      if (pRootnode->Child[i]) {
        forEachNode(pRootnode->Child[i], func);
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

    if ((P->depth - tempPtr->depth) > 1 && (tempPtr->type == Node::LEAF_TYPE)) {

      constructTreeNode(tempPtr);
      for (unsigned int j = 0; j < 4; ++j) {
        balanceTree(tempPtr->Child[j]);
      }
    }
  }
}

void recursive(Node *pNode, int D1, int D2, std::vector<Node *> &vector) {

  if (pNode->type == Node::LEAF_TYPE) {
    vector.push_back(pNode);
  }

  else {
    recursive(pNode->Child[D1], D1, D2, vector);
    recursive(pNode->Child[D2], D1, D2, vector);
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
      pNeighbourNode = findTreeNode(thisNode->centre_x() + (thisNode->x_dsp() + (thisNode->x_dsp(maxGridDepth) / 2.0)),
                                    thisNode->centre_y());

      // If the neighbouring node is not this node (flaw in neighbour finding function)
      if (pNeighbourNode->id != thisNode->id) {
        // traverse through the neighbouring node parents until the parent with the same depth is found
        while (pNeighbourNode->depth > thisNode->depth) {
          pNeighbourNode = pNeighbourNode->Parent;
        }

        // Now find all the NW and SW children of the neighbouring node that share the same edge with this node and add
        // the to a container
        recursive(pNeighbourNode, NW, SW, vector);
      }

      break;

    case W:
      pNeighbourNode = findTreeNode(thisNode->centre_x() - (thisNode->x_dsp() + (thisNode->x_dsp(maxGridDepth) / 2.0)),
                                    thisNode->centre_y());

      if (pNeighbourNode->id != thisNode->id) {
        while (pNeighbourNode->depth > thisNode->depth) {
          pNeighbourNode = pNeighbourNode->Parent;
        }
        recursive(pNeighbourNode, NE, SE, vector);
      }

      break;

    case N:
      pNeighbourNode =
          findTreeNode(thisNode->centre_x(),
                       thisNode->centre_y() - (thisNode->y_dsp() + (thisNode->y_dsp(maxGridDepth) / 2.0)) + 0.005);

      if (pNeighbourNode->id != thisNode->id) {
        while (pNeighbourNode->depth > thisNode->depth) {
          pNeighbourNode = pNeighbourNode->Parent;
        }
        recursive(pNeighbourNode, SW, SE, vector);
      }

      break;

    case S:
      pNeighbourNode = findTreeNode(thisNode->centre_x(),
                                    thisNode->centre_y() + (thisNode->y_dsp() + (thisNode->y_dsp(maxGridDepth) / 2.0)));
      if (pNeighbourNode->id != thisNode->id) {
        while (pNeighbourNode->depth > thisNode->depth) {
          pNeighbourNode = pNeighbourNode->Parent;
        }
        recursive(pNeighbourNode, NW, NE, vector);
      }
      break;

    case NW:
      pNeighbourNode = findTreeNode(thisNode->centre_x() - (thisNode->x_dsp() + (thisNode->x_dsp(maxGridDepth) / 2.0)),
                                    thisNode->centre_y() - (thisNode->y_dsp() + (thisNode->y_dsp(maxGridDepth) / 2.0)));
      if (pNeighbourNode->id != thisNode->id) {
        vector.push_back(pNeighbourNode);
      }
      break;

    case NE:
      pNeighbourNode = findTreeNode(thisNode->centre_x() + (thisNode->x_dsp() + (thisNode->x_dsp(maxGridDepth) / 2.0)),
                                    thisNode->centre_y() - (thisNode->y_dsp() + (thisNode->y_dsp(maxGridDepth) / 2.0)));
      if (pNeighbourNode->id != thisNode->id) {
        vector.push_back(pNeighbourNode);
      }
      break;

    case SW:
      pNeighbourNode = findTreeNode(thisNode->centre_x() - (thisNode->x_dsp() + (thisNode->x_dsp(maxGridDepth) / 2.0)),
                                    thisNode->centre_y() + (thisNode->y_dsp() + (thisNode->y_dsp(maxGridDepth) / 2.0)));
      if (pNeighbourNode->id != thisNode->id) {
        vector.push_back(pNeighbourNode);
      }
      break;

    case SE:
      pNeighbourNode = findTreeNode(thisNode->centre_x() + (thisNode->x_dsp() + (thisNode->x_dsp(maxGridDepth) / 2.0)),
                                    thisNode->centre_y() + (thisNode->y_dsp() + (thisNode->y_dsp(maxGridDepth) / 2.0)));
      if (pNeighbourNode->id != thisNode->id) {
        vector.push_back(pNeighbourNode);
      }
      break;
    }
  }

  // Remove duplicates from the vector

  // Predicates lambda functions
  auto compare_nodes = [](Node *i, Node *j) -> bool { return i->id == j->id; };
  auto sort_nodes = [](Node *i, Node *j) -> bool { return i->id < j->id; };

  // Sort first
  std::sort(vector.begin(), vector.end(), sort_nodes);

  // then remove duplicates
  vector.erase(std::unique(vector.begin(), vector.end(), compare_nodes), vector.end());
}

// Find Neighbouring node using geometric coordinates, return leaf node
Node *QuadTree::findLeafNeighbour(Node *p, int Direction) {
  switch (Direction) {
  case E:
    return findTreeNode(p->centre_x() + (p->x_dsp() + (p->x_dsp(maxGridDepth) / 2.0)), p->centre_y());
    break;

  case W:
    return findTreeNode(p->centre_x() - (p->x_dsp() + (p->x_dsp(maxGridDepth) / 2.0)), p->centre_y());
    break;

  case N:

    return findTreeNode(p->centre_x(), p->centre_y() - (p->y_dsp() + (p->y_dsp(maxGridDepth) / 2.0)));
    break;

  case S:
    return findTreeNode(p->centre_x(), p->centre_y() + (p->y_dsp() + (p->y_dsp(maxGridDepth) / 2.0)));
    break;

  case NW:
    return findTreeNode(p->centre_x() - (p->x_dsp() + (p->x_dsp(maxGridDepth) / 2.0)),
                        p->centre_y() - (p->y_dsp() + (p->y_dsp(maxGridDepth) / 2.0)));
    break;

  case NE:
    return findTreeNode(p->centre_x() + (p->x_dsp() + (p->x_dsp(maxGridDepth) / 2.0)),
                        p->centre_y() - (p->y_dsp() + (p->y_dsp(maxGridDepth) / 2.0)));
    break;

  case SW:
    return findTreeNode(p->centre_x() - (p->x_dsp() + (p->x_dsp(maxGridDepth) / 2.0)),
                        p->centre_y() + (p->y_dsp() + (p->y_dsp(maxGridDepth) / 2.0)));
    break;

  case SE:
    return findTreeNode(p->centre_x() + (p->x_dsp() + (p->x_dsp(maxGridDepth) / 2.0)),
                        p->centre_y() + (p->y_dsp() + (p->y_dsp(maxGridDepth) / 2.0)));
    break;

  default:
    throw std::runtime_error("Unknown Direction");
  };
}

// Find Neighbours using pointer relationship, FAST operation, but return node at the same depth, ignoring child nodes
Node *QuadTree::findNeighbour(Node *p, int Direction) {
  if (p == this->m_rootNode) {
    // no neighbour for the parent node
    return this->m_rootNode;
  }

  // NW, SW, NE, SE
  switch (Direction) {
  case E:

    // if p = NW-child
    if (p == p->Parent->Child[NW]) {
      // then return the NE
      return p->Parent->Child[NE];
    }

    // if p = SW-child
    else if (p == p->Parent->Child[SW]) {
      // then return the SE
      return p->Parent->Child[SE];
    }
    break;

  case W:

    // if p = NW-child
    if (p == p->Parent->Child[NE]) {
      // then return the NE
      return p->Parent->Child[NW];
    }

    // if p = SW-child
    else if (p == p->Parent->Child[SE]) {
      // then return the SE
      return p->Parent->Child[SW];
    }
    break;

  case N:

    // if p = NW-child
    if (p == p->Parent->Child[SE]) {
      // then return the NE
      return p->Parent->Child[NE];
    }

    // if p = SW-child
    else if (p == p->Parent->Child[SW]) {
      // then return the SE
      return p->Parent->Child[NW];
    }
    break;

  case S:

    // if p = NW-child
    if (p == p->Parent->Child[NE]) {
      // then return the NE
      return p->Parent->Child[SE];
    }

    // if p = SW-child
    else if (p == p->Parent->Child[NW]) {
      // then return the SE
      return p->Parent->Child[SW];
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
  Node *p1 = findNeighbour(p->Parent, Direction);

  if (p1->type == Node::LEAF_TYPE || p1->type == Node::ROOT_TYPE) {
    return p1;
  }

  else {
    switch (Direction) {
    case E:
      // if p == NE-Child
      if (p == p->Parent->Child[NE]) {
        // return the NW-Child of p1
        return p1->Child[NW];
      }

      else {
        // return the SW-Child of p1
        return p1->Child[SW];
      }
      break;

    case W:
      // if p == NE-Child
      if (p == p->Parent->Child[NW]) {
        // return the NW-Child of p1
        return p1->Child[NE];
      }

      else {
        // return the SW-Child of p1
        return p1->Child[SE];
      }
      break;

    case N:
      // if p == NE-Child
      if (p == p->Parent->Child[NW]) {
        // return the NW-Child of p1
        return p1->Child[SW];
      }

      else {
        // return the SW-Child of p1
        return p1->Child[SE];
      }
      break;

    case S:
      // if p == NE-Child
      if (p == p->Parent->Child[SW]) {
        // return the NW-Child of p1
        return p1->Child[NW];
      }

      else {
        // return the SW-Child of p1
        return p1->Child[NE];
      }
      break;
    }
  }
  throw std::runtime_error("Could not find a neighbour");
}

Node *QuadTree::findNeighbour(Node *p, char Direction) {
  boost::dynamic_bitset<> xbit;
  boost::dynamic_bitset<> ybit;

  while (p->type != Node::ROOT_TYPE) {
    qDebug() << p->centre_x() << " - x - " << p->Parent->centre_x() << endl;
    qDebug() << p->centre_y() << " - y - " << p->Parent->centre_y() << endl;

    if (p->centre_x() > p->Parent->centre_x()) {
      xbit.push_back(1);
    }

    else {
      xbit.push_back(0);
    }

    if (p->centre_y() > p->Parent->centre_y()) {
      ybit.push_back(1);
    }

    else {
      ybit.push_back(0);
    }

    p = p->Parent;
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
  // Get dX, dY,dZ
  P3 temp_p3;
  temp_p3.x = start->centre_x() - finish->centre_x();
  temp_p3.y = start->centre_y() - finish->centre_y();
  // Calculate the distance
  return sqrt((temp_p3.x * temp_p3.x) + (temp_p3.y * temp_p3.y));
}

std::vector<Node *> construct_path(std::vector<Node *>, Node *finish);

bool sortByCost(std::pair<int, Node *> i, std::pair<int, Node *> j) {
  return ((i.second->f_cost) < (j.second->f_cost));
}

Node *getNodeWithLowestCost(std::map<int, Node *> &map) {
  //*std::min_element (map.begin(), map.end(), sortByCost);

  return std::min_element(map.begin(), map.end(), sortByCost)->second;
}

bool QuadTree::path_routing(Node *start, Node *finish) {

  //	forEachNode ( rootNode, std::bind(&QuadTree::balanceTree, this, std::placeholders::_1) );
  Node *current;
  closedSet.clear();
  camefromSet.clear();
  openedSet.clear();

  openedSet[start->id] = start;
  start->cost = 0;
  start->f_cost = 0;
  // start->A_Parent = start;

  while (!openedSet.empty()) {
    // getNodeWithLowestCost( openedSet)->A_Parent = current;
    current = getNodeWithLowestCost(openedSet);

    qDebug() << "Node with lowest cost is: " << current->id;
    // Node Found!
    if (current == finish) {
      qDebug() << "Path found";

      Node *tnode = finish;
      camefromSet.clear();

      camefromSet.push_back(tnode);

      while (tnode->id != start->id) {
        camefromSet.push_back(tnode);
        tnode = tnode->A_Parent;
      }
      camefromSet.push_back(start);

      return true;
      //	construct_path( camefromSet, finish);
    }

    else {
      openedSet.erase(current->id);
      closedSet[current->id] = current;

      // for each neighbour

      getAllNeighbours(current, neighbours);

      // Initialize for node scanning
      double tentative_cost = 0;
      auto neighbour_itr = neighbours.begin();
      while (neighbour_itr != neighbours.end()) {
        tentative_cost = current->cost + distance(current, (*neighbour_itr));

        qDebug() << "tentative cost: " << tentative_cost << " nbr_node: " << (*neighbour_itr)->id;
        // if the node in the closed list
        if (closedSet.find((*neighbour_itr)->id) != closedSet.end() /*&& neighbour_itr->depth != 4*/) {
          if (tentative_cost >= (*neighbour_itr)->cost) {
            ++neighbour_itr;
            continue;
          }
        }

        // if the node not in the opened list
        if (openedSet.find((*neighbour_itr)->id) == openedSet.end() || tentative_cost < (*neighbour_itr)->cost) {
          // add more statements (node not walkable)
          (*neighbour_itr)->A_Parent = current;
          (*neighbour_itr)->cost = tentative_cost;
          (*neighbour_itr)->f_cost = tentative_cost + distance((*neighbour_itr), finish);
        }

        // if the node not in the closed list
        if (closedSet.find((*neighbour_itr)->id) == closedSet.end() /*&& neighbour_itr->depth != 4*/) {
          // add neighbour to openset
          openedSet[(*neighbour_itr)->id] = (*neighbour_itr);
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
