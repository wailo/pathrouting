#include "GLWidget.h"
#include <QOpenGLShaderProgram>
#include <QtDebug>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <array>

static const char *fragmentShaderSourceCore = R"(#version 330 core
out vec3 color;
void main(){
  color = vec3(0,2,0);
})";

static const char *vertexShaderSourceCore = R"(#version 150
                                            in vec4 vertex;
                                            in vec3 normal;
                                            out vec3 vert;
                                            out vec3 vertNormal;
                                            uniform mat4 projMatrix;
                                            uniform mat4 mvMatrix;
                                            uniform mat3 normalMatrix;
                                            void main() {
                                               vert = vertex.xyz;
                                               vertNormal = normalMatrix * normal;
                                               gl_Position = projMatrix * mvMatrix * vertex;
                                            })";

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent), Tree(new QuadTree(0, 2048, 0, 2048)), m_zoom_factor(1.0), m_program(0) {
  setMouseTracking(true);

  QSurfaceFormat glFormat;
  glFormat.setVersion(4, 1);
  glFormat.setProfile(QSurfaceFormat::CoreProfile);
  // glFormat.setSampleBuffers(true);
  glFormat.setDefaultFormat(glFormat);
  glFormat.setSwapInterval(1);
  setFormat(glFormat);
  create();

  // Read Data
  bool OK = false;
  OK = data.read_AIXM_file(R"(../Airport_data/Chicago_Ohare/Chicago O'Hare Aprons_CRS84.xml)") &&
       data.read_AIXM_file(R"(../Airport_data/Chicago_Ohare/Chicago O'Hare Taxiways_CRS84.xml)") &&
       data.read_AIXM_file(R"(../Airport_data/Chicago_Ohare/Chicago O'Hare Runways_CRS84.xml)") &&
       data.read_AIXM_file(R"(../Airport_data/Chicago_Ohare/Chicago O'Hare VerticalStructures_CRS84.xml)");
  // data.read_AIXM_file( "..//Airport_data//Chicago O'Hare Taxiways.xml");
  // data.read_AIXM_file( "..//Airport_data//Chicago O'Hare Runways.xml");

  if (!OK) {
    return;
  }

  data.process_boundaries(*Tree);
  Tree->balanceTree(Tree->m_rootNode);
}

void GLWidget::initializeGL() {

  initializeOpenGLFunctions();

  glClearColor(0, 0, 0, 0);
  m_program = new QOpenGLShaderProgram;

  bool res = m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSourceCore);
  res = m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSourceCore);
  m_program->bindAttributeLocation("vertex", 0);
  m_program->bindAttributeLocation("color", 1);
  res = m_program->link();

  m_program->bind();
  m_projMatrixLoc = m_program->uniformLocation("projMatrix");
  m_mvMatrixLoc = m_program->uniformLocation("mvMatrix");
  m_normalMatrixLoc = m_program->uniformLocation("normalMatrix");
  // m_lightPosLoc = m_program->uniformLocation("lightPos");

  generate_grid_vertices(Tree->m_rootNode, vertex_list);
  generate_airport_vertices(vertex_list);

  m_vao.create();
  QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

  // Setup our vertex buffer object.
  m_logoVbo.create();
  m_logoVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  m_logoVbo.bind();
  m_logoVbo.allocate(vertex_list.data(), vertex_list.size() * sizeof(GLdouble));
  m_logoVbo.bind();

  // Store the vertex attribute bindings for the program.
  setupVertexAttribs();
  m_program->release();

  // Print debug data
  const QOpenGLContext *m_context = context();
  qDebug() << "Context valid: " << m_context->isValid();
  qDebug() << "Really used OpenGl: " << m_context->format().majorVersion() << "." << m_context->format().minorVersion();
  int major = 0;
  int minor = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  qDebug() << major << " " << minor;
  qDebug() << (char *)glGetString(GL_VERSION);
  qDebug() << "OpenGl information: VENDOR:       " << (const char *)glGetString(GL_VENDOR);
  qDebug() << "                    RENDERDER:    " << (const char *)glGetString(GL_RENDERER);
  qDebug() << "                    VERSION:      " << (const char *)glGetString(GL_VERSION);
  qDebug() << "                    GLSL VERSION: " << (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
  qDebug() << "endstuff\n";
}

void GLWidget::setupVertexAttribs() {
  QOpenGLFunctions *f = context()->functions();
  f->glEnableVertexAttribArray(0);
  f->glEnableVertexAttribArray(1);
  f->glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(GLdouble), 0);
  f->glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(GLdouble), 0);
  m_logoVbo.release();
}

void GLWidget::resizeGL(int w, int h) {
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  m_width = w;
  m_height = h;
  // Tree->invalidate_draw();
  m_proj.setToIdentity();
  // m_proj.ortho(-1.1, 1.1, -1.1, 1.1, -1, 1);
  m_proj.ortho(Tree->get_left() - 0.001, Tree->get_right() + 0.001, Tree->get_bottom() - 0.001, Tree->get_top() + 0.001, -1,
               1);
}

void GLWidget::paintGL() {

  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  m_world.setToIdentity();
  m_world.rotate(0, 1, 0, 0);
  m_world.rotate(m_yRot / 16.0f, 0, 1, 0);
  m_world.rotate(m_zRot / 16.0f, 0, 0, 1);

  QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
  m_program->bind();
  m_program->setUniformValue(m_projMatrixLoc, m_proj);
  m_program->setUniformValue(m_mvMatrixLoc, m_camera * m_world);
  QMatrix3x3 normalMatrix = m_world.normalMatrix();
  m_program->setUniformValue(m_normalMatrixLoc, normalMatrix);
  glDrawArrays(GL_LINES, 0, vertex_list.size() / 3);
  m_program->release();

  // for (const auto& object : data.Objects) {

  //   if ((object)->m_AIXM_object_type.compare("GuidanceLine") == 0) {
  //     glColor3f(0, 1, 0);
  //   } else if ((object)->m_AIXM_object_type.compare("TaxiwayElement") == 0) {
  //     glColor3f(1, 1, 0);
  //   } else if ((object)->m_AIXM_object_type.compare("RunwayElement") == 0) {
  //     glColor3f(0, 0, 1);
  //   } else {
  //   }

  //   glPointSize(3);
  //   glBegin(GL_LINE_STRIP);
  //   for (const auto& coord : object->m_Coordinates) {
  //     glVertex3f(coord.m_Lon, coord.m_Lat, 0);
  //   }
  //   glEnd();
  // }

  // // Draw the shortest path
  // glColor4f(0, 1, 0, 0.5);
  // for (const auto& node :Tree->camefromSet) {
  //   glBegin(GL_POLYGON);
  //   glVertex3f(node->centre_x() - (node->x_dsp()), node->centre_y() + (node->y_dsp()), 0);
  //   glVertex3f(node->centre_x() - (node->x_dsp()), node->centre_y() - (node->y_dsp()), 0);
  //   glVertex3f(node->centre_x() + (node->x_dsp()), node->centre_y() - (node->y_dsp()), 0);
  //   glVertex3f(node->centre_x() + (node->x_dsp()), node->centre_y() + (node->y_dsp()), 0);
  //   glEnd();
  // }

  // Tree->camefromSet.clear();
}

void GLWidget::generate_grid_vertices(const Node *pNode, std::vector<GLdouble> &list) {

  if (!pNode) {
    return;
  }

  list.insert(list.end(), {(pNode->centre_x() - pNode->x_dsp()), (pNode->centre_y() + pNode->y_dsp()), 0,
                           (pNode->centre_x() - pNode->x_dsp()), (pNode->centre_y() - pNode->y_dsp()), 0,

                           (pNode->centre_x() - pNode->x_dsp()), (pNode->centre_y() - pNode->y_dsp()), 0,
                           (pNode->centre_x() + pNode->x_dsp()), (pNode->centre_y() - pNode->y_dsp()), 0,

                           (pNode->centre_x() + pNode->x_dsp()), (pNode->centre_y() - pNode->y_dsp()), 0,
                           (pNode->centre_x() + pNode->x_dsp()), (pNode->centre_y() + pNode->y_dsp()), 0,

                           (pNode->centre_x() + pNode->x_dsp()), (pNode->centre_y() + pNode->y_dsp()), 0,
                           (pNode->centre_x() - pNode->x_dsp()), (pNode->centre_y() + pNode->y_dsp()), 0});

  for (auto &child : pNode->Child) {
    generate_grid_vertices(child, list);
  }
}

void GLWidget::generate_airport_vertices(std::vector<GLdouble> &list) {
  for (const auto &object : data.Objects) {

    // if ((object)->m_AIXM_object_type.compare("GuidanceLine") == 0) {
    //   glColor3f(0, 1, 0);
    // } else if ((object)->m_AIXM_object_type.compare("TaxiwayElement") == 0) {
    //   glColor3f(1, 1, 0);
    // } else if ((object)->m_AIXM_object_type.compare("RunwayElement") == 0) {
    //   glColor3f(0, 0, 1);
    // } else {
    // }

    for (std::size_t i = 1; i < object->m_Coordinates.size(); ++i) {
      list.insert(list.end(), {object->m_Coordinates.at(i - 1).m_Lon, object->m_Coordinates.at(i - 1).m_Lat, 0});
      list.insert(list.end(), {object->m_Coordinates.at(i).m_Lon, object->m_Coordinates.at(i).m_Lat, 0});
    }
  }
}

void GLWidget::mousePressEvent(QMouseEvent *event) {

  QVector3D worldPosition = QVector3D(event->x() * devicePixelRatioF(), event->y() * devicePixelRatioF(), 0)
                                .unproject(m_world, m_proj, QRect(0, 0, width(), height()));

  switch (event->button()) {
  case Qt::LeftButton:

    if (QApplication::keyboardModifiers() == Qt::ALT) {

      if (testVec.size() < 2) {
        qDebug() << "Pathrouting S" << testVec.size();
        testVec.push_back(Tree->findTreeNode(worldPosition.x(), worldPosition.y()));
      } else {
        Tree->path_routing(testVec.at(0), testVec.at(1));
        testVec.clear();
      }
    }

    else {
      Tree->constructTreeNode(Tree->findTreeNode(worldPosition.x(), worldPosition.y()));
    }

    break;

  case Qt::RightButton:
    // Wrong! because findTreeNode a create a node as well, modify it so it only find the node and return pointer to it
    // assign separate function the called separatly to create a node
    // Tree->removeTreeNode( Tree->findTreeNode( event->x() , event->y() )->Parent );
    // Tree->constructTreeNode( Tree->findNeighbour( Tree->findTreeNode( event->x() , event->y()) , NULL ) );
    // Tree->getAllNeighbours(Tree->findTreeNode( event->x() , event->y()), Tree->neighbours );

    Tree->forEachNode(Tree->m_rootNode, [&](Node *node) { Tree->balanceTree(node); });

    break;
  default:
    event->ignore();
    break;
  }
  update();
}
void GLWidget::mouseMoveEvent(QMouseEvent *event) {
  //	printf("%d, %d\n", event->x(), event->y());
  // if (event->buttons() & Qt::LeftButton) {

  // code here
  // }
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_Escape:
    QCoreApplication::quit();
    break;
  default:
    event->ignore();
    break;
  }
}

void GLWidget::wheelEvent(QWheelEvent *event) {
  int numDegrees = event->delta() / 8.0;
  int numSteps = numDegrees / 15;
  m_zoom_factor += numSteps * 0.01;
  resizeGL(m_width, m_height);

  if (event->orientation() == Qt::Horizontal) {
    // scrollHorizontally(numSteps);
  } else {

    // zoomIn(event->x(),event->y(),1+(numSteps*.01));
    // scrollVertically(numSteps);
  }
  event->accept();
}

void GLWidget::resetTree() {
  Tree->removeTreeNode(Tree->m_rootNode);
  glClear(GL_COLOR_BUFFER_BIT);
  update();
}
