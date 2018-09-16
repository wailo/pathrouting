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

// static const char *fragmentShaderSourceCore = "#version 150\n"
//                                               "in highp vec3 vert;\n"
//                                               "in highp vec3 vertNormal;\n"
//                                               "out highp vec4 fragColor;\n"
//                                               "uniform highp vec3 lightPos;\n"
//                                               "void main() {\n"
//                                               "   highp vec3 L = normalize(lightPos - vert);\n"
//                                               "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
//                                               "   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
//                                               "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
//                                               "   fragColor = vec4(col, 1.0);\n"
//                                               "}\n";

static const char *vertexShaderSource = "attribute vec4 vertex;\n"
                                        "attribute vec3 normal;\n"
                                        "varying vec3 vert;\n"
                                        "varying vec3 vertNormal;\n"
                                        "uniform mat4 projMatrix;\n"
                                        "uniform mat4 mvMatrix;\n"
                                        "uniform mat3 normalMatrix;\n"
                                        "void main() {\n"
                                        "   vert = vertex.xyz;\n"
                                        "   vertNormal = normalMatrix * normal;\n"
                                        "   gl_Position = projMatrix * mvMatrix * vertex;\n"
                                        "}\n";

static const char *fragmentShaderSource = "varying highp vec3 vert;\n"
                                          "varying highp vec3 vertNormal;\n"
                                          "uniform highp vec3 lightPos;\n"
                                          "void main() {\n"
                                          "   highp vec3 L = normalize(lightPos - vert);\n"
                                          "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
                                          "   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
                                          "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
                                          "   gl_FragColor = vec4(col, 1.0);\n"
                                          "}\n";

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

  std::vector<std::array<double, 3>> vertices;
  Tree->forEachNode(Tree->m_rootNode, [&vertices](Node *p_Node) {
    std::array<double, 3> p;
    double arr[3] = {p_Node->centre_x(), p_Node->centre_y(), p_Node->centre_z()};
    p[0] = arr[0];
    p[1] = arr[1];
    p[2] = arr[2];
    vertices.push_back(p);
  });

  // implement display list here!
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
  m_vao.create();
  QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

  // Setup our vertex buffer object.
  m_logoVbo.create();
  m_logoVbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
  m_logoVbo.bind();
  m_logoVbo.allocate(vertex_list.data(), vertex_list.size() * sizeof(GLfloat));
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
  f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  m_logoVbo.release();
}


void GLWidget::resizeGL(int w, int h) {
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  m_width = w;
  m_height = h;
  Tree->invalidate_draw();
  m_proj.setToIdentity();
  // m_proj.ortho(-1.1, 1.1, -1.1, 1.1, -1, 1);
  m_proj.ortho(Tree->get_left(), Tree->get_right(), Tree->get_bottom(), Tree->get_top(), -1, 1);
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
  glDrawArrays(GL_LINES, 0, vertex_list.size()/3);
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


void GLWidget::generate_grid_vertices(const Node *pNode, std::vector<GLfloat> &list) {
  
  if (!pNode) {
    return;
  }

  list.push_back(pNode->centre_x() - pNode->x_dsp());  list.push_back(pNode->centre_y() + pNode->y_dsp());  list.push_back(0);
  list.push_back(pNode->centre_x() - pNode->x_dsp());  list.push_back(pNode->centre_y() - pNode->y_dsp());  list.push_back(0);
  
  list.push_back(pNode->centre_x() - pNode->x_dsp());  list.push_back(pNode->centre_y() - pNode->y_dsp());  list.push_back(0);
  list.push_back(pNode->centre_x() + pNode->x_dsp());  list.push_back(pNode->centre_y() - pNode->y_dsp());  list.push_back(0);
  
  list.push_back(pNode->centre_x() + pNode->x_dsp());  list.push_back(pNode->centre_y() - pNode->y_dsp());  list.push_back(0);
  list.push_back(pNode->centre_x() + pNode->x_dsp());  list.push_back(pNode->centre_y() + pNode->y_dsp());  list.push_back(0);
  
  list.push_back(pNode->centre_x() + pNode->x_dsp());  list.push_back(pNode->centre_y() + pNode->y_dsp());  list.push_back(0);
  list.push_back(pNode->centre_x() - pNode->x_dsp());  list.push_back(pNode->centre_y() + pNode->y_dsp());  list.push_back(0);

  for (auto &child : pNode->Child) {
    generate_grid_vertices(child, list);
  }
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  double world_x, world_y, world_z;
  double model_view[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

  double projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  gluUnProject(event->x() * this->devicePixelRatioF(), viewport[3] - event->y() * this->devicePixelRatioF(), 0.0,
               model_view, projection, viewport, &world_x, &world_y, &world_z);

  switch (event->button()) {
  case Qt::LeftButton:

    if (QApplication::keyboardModifiers() == Qt::ALT) {

      if (testVec.size() < 2) {
        qDebug() << "Pathrouting S" << testVec.size();
        testVec.push_back(Tree->findTreeNode(world_x, world_y));
      } else {
        Tree->path_routing(testVec.at(0), testVec.at(1));
        testVec.clear();
      }
    }

    else {
      Tree->constructTreeNode(Tree->findTreeNode(world_x, world_y));
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
