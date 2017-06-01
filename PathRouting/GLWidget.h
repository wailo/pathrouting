#pragma once

#include <Windows.h>

#include <gl\GL.h>
#include <gl\GLU.h>
#include <QtOpenGL/QGLWidget>
#include "QuadTree.h"
#include "AIXM_file_parser.h"

class GLWidget : public QGLWidget {

  Q_OBJECT // must include this if you use Qt signals/slots

 public:

  GLWidget(QWidget *parent = NULL);
  QuadTree *Tree;

  // Draw Nodes
  void drawTreeNode (Node* p);

  // for routePath test
  std::vector<Node*> testVec;

  AIXM_file_parser data;
  

 protected:
  //int width, height;
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *);
  void wheelEvent( QWheelEvent *event);

  void zoom( const int& p_x, const int& p_y, const double& p_factor );
  void zoomOut();
  void setViewingVolume(const int& p_x, const int& p_y, const double& p_zoom_factor );

 public slots:
  void resetTree();
 signals:
  void TreeChanged( const QString& NodeCount);

 private:

  double m_zoom_factor;
  int m_width, m_height;
};

