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
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <memory>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {

  Q_OBJECT // must include this if you use Qt signals/slots

      public :

      GLWidget(QWidget *parent = NULL);
  std::unique_ptr<QuadTree> Tree;

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

  int m_xRot{0};
  int m_yRot{0};
  int m_zRot{0};
  int m_projMatrixLoc;
  int m_mvMatrixLoc;
  int m_normalMatrixLoc;
  int m_lightPosLoc;
  QMatrix4x4 m_proj;
  QMatrix4x4 m_camera;
  QMatrix4x4 m_world;

  struct vertex_object {
    GLdouble x{0}, y{0}, z{0}, r{0}, g{0}, b{0};
  };

  std::vector<vertex_object> m_vertex_list;
  
  std::vector<GLsizei> m_airport_vertex_count;
  std::vector<GLint> m_airport_vertex_indices;
  
  std::vector<GLsizei> m_grid_vertex_count;
  std::vector<GLint> m_grid_vertex_indices;

  // Draw Nodes
  void generate_grid_vertices(const Node *pNode, std::vector<vertex_object> &list);
  void generate_airport_vertices(std::vector<vertex_object> &list);
  void setupVertexAttribs();
};
