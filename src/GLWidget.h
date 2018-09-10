#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#elif __APPLE__
#include <gl.h>
#include <glu.h>
#endif

 #include "AIXM_file_parser.h"
 #include "QuadTree.h"
#include <QtOpenGL/QGLWidget>
#include <memory>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
class GLWidget : public QGLWidget, protected QOpenGLFunctions {

  Q_OBJECT // must include this if you use Qt signals/slots

      public :

      GLWidget(QWidget *parent = NULL);
  std::unique_ptr<QuadTree> Tree;

  
  void generate_vbo(const Node *pNode, std::vector<GLfloat>& list);
  
  void setupVertexAttribs();
  // Draw Nodes
  void drawTreeNode(Node *p);

  // for routePath test
  std::vector<Node *> testVec;

  AIXM_file_parser data;

protected:
  // int width, height;
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *);
  void wheelEvent(QWheelEvent *event);

  void zoom(const int &p_x, const int &p_y, const double &p_factor);
  void zoomOut();
  void setViewingVolume(const int &p_x, const int &p_y, const double &p_zoom_factor);

public slots:
  void resetTree();
signals:
  void TreeChanged(const QString &NodeCount);

private:
  double m_zoom_factor;
  int m_width, m_height;

  QOpenGLVertexArrayObject m_vao;
  QOpenGLBuffer m_logoVbo;
  QOpenGLShaderProgram *m_program;
  GLuint vao = 0;
  GLuint points_vbo = 0;

  std::vector<GLfloat> vertex_list;

};
