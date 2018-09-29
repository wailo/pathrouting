#pragma once

#include "Node.h"
#include <algorithm>
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <functional>
#include <map>
#include <unordered_map>

class QuadTree {

private:
  // Max node depth, this value increments when a node with new depth level is created.
  static int maxGridDepth;

  void InitRootNode();

  // Reset draw flag
  void invalidate_draw(Node *node);

  //--- Path Rouring ---//

  // Closed set container
  std::map<int, Node *> closedSet;

  // Opened set container
  std::map<int, Node *> openedSet;

  // Copy container for neighbour scanning
  std::vector<Node *> neighbours;

  // distance (cost) calculation
  double distance(Node *start, Node *end);

  //--- End of Path Routing ---//

public:
  // BOunding box for the tree
  double right, left, bottom, top;

  // enfore the order of nodes in a quad using bools
  // for h_order 0= left, 1 =right
  // for v_order 0= top, 1= bottom
  bool h_order[4];
  bool v_order[4];

  // Width of the root node
  double gridWidth;
  // Height of the root node
  double gridHeight;

  // Remove node from the tree
  void removeTreeNode(Node &pNode);

  // Tree Ctor with boundary arguments
  QuadTree(int left, int right, int bottom, int top);

  // Dtor
  ~QuadTree(void);

  // Root node of the tree
  Node m_rootNode;

  // Retrieve Tree node that contains XY coordinate
  Node *findTreeNode(double x, double y, Node *p_startnode = NULL);

  // Construct a tree branch by constructing all the children nodes
  void constructTreeNode(Node *node);

  // Enforce [2:1] rule between cells
  void balanceTree(Node *P);

  // Upon windows resize, call this function to update the tree boundary
  void updateTreeBoundary(double left, double right, double bottom, double top);

  // Node visitor function. Visit all the tree nodes and execute func
  void forEachNode(Node *pRootnode, std::function<void(Node *)> func);

  // void updateNodeCentre ( Node* node );

  // get the number of nodes created
  int getNodeCount() const;

  // Getters
  double get_right();
  double get_left();
  double get_bottom();
  double get_top();

  // Find neighbour node with the same depth
  Node *findNeighbour(Node *p, int Direction);
  Node *findNeighbour(Node *p, char Direction);

  // Find neighbour node with LEAF type (lowest level)
  Node *findLeafNeighbour(Node *p, int Direction);

  // get all the neighbours of a selected node
  void getAllNeighbours(Node *p, std::vector<Node *> &);

  // Reset draw flag
  void invalidate_draw();

  enum Directions { NW, SW, NE, SE, N, S, E, W };

  // A* Algorithm Function
  bool path_routing(Node *start, Node *finish);

  // Vector the store the nodes that represent the shortes path.
  std::vector<Node *> camefromSet;
};
