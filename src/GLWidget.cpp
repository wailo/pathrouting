#include "GLWidget.h"
#include <QOpenGLShaderProgram>
#include <QtDebug>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <array>

// #include <GLFW/glfw3.h>

static const char *vertexShaderSourceCore = "#version 150\n"
                                            "in vec4 vertex;\n"
                                            "in vec3 normal;\n"
                                            "out vec3 vert;\n"
                                            "out vec3 vertNormal;\n"
                                            "uniform mat4 projMatrix;\n"
                                            "uniform mat4 mvMatrix;\n"
                                            "uniform mat3 normalMatrix;\n"
                                            "void main() {\n"
                                            "   vert = vertex.xyz;\n"
                                            "   vertNormal = normalMatrix * normal;\n"
                                            "   gl_Position = projMatrix * mvMatrix * vertex;\n"
                                            "}\n";

static const char *fragmentShaderSourceCore = "#version 150\n"
                                              "in highp vec3 vert;\n"
                                              "in highp vec3 vertNormal;\n"
                                              "out highp vec4 fragColor;\n"
                                              "uniform highp vec3 lightPos;\n"
                                              "void main() {\n"
                                              "   highp vec3 L = normalize(lightPos - vert);\n"
                                              "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
                                              "   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
                                              "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
                                              "   fragColor = vec4(col, 1.0);\n"
                                              "}\n";

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

  // m_context->create();
  // m_context->makeCurrent();
  // context
  // m_context->makeCurrent();
  // m_context->makeCurrent(this);
  // this->makeCurrent(this);
  // setContext(m_context);

  // Make the context current on this window
  // m_context->makeCurrent( this );

  // Obtain a functions object and resolve all entry points
  // m_funcs is declared as: QOpenGLFunctions_4_3_Core* m_funcs
  // auto m_funcs = m_context->i versionFunctions();
  // if ( !m_funcs ) {
  //     qWarning( &quot;Could not obtain OpenGL versions object&quot; );
  //     exit( 1 );
  // }
  // m_funcs->initializeOpenGLFunctions();
  // glewExperimental = GL_TRUE;
  // if (glewInit() != GLEW_OK) {
  //     qWarning("Failed to initialize GLEW\n");
  // }

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

  glClearColor(0, 0, 0, 0);
  m_program = new QOpenGLShaderProgram;

  bool res = m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSourceCore);
  res = m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSourceCore);
  m_program->bindAttributeLocation("vertex", 0);
  m_program->bindAttributeLocation("normal", 1);
  res = m_program->link();

  res = m_program->bind();
  res = m_projMatrixLoc = m_program->uniformLocation("projMatrix");
  res = m_mvMatrixLoc = m_program->uniformLocation("mvMatrix");
  res = m_normalMatrixLoc = m_program->uniformLocation("normalMatrix");
  res = m_lightPosLoc = m_program->uniformLocation("lightPos");

  //	setAutoBufferSwap( TRUE );
  //	glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  //	glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_BLEND);
  //	glEnable(GL_POLYGON_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClear(GL_COLOR_BUFFER_BIT);

  generate_vbo(Tree->m_rootNode, vertex_list);
  decltype(vertex_list) vertex_color(vertex_list.size());
  std::fill(vertex_color.begin(), vertex_color.end(), 1);

  GLuint points_vbo = 0;
  GLfloat *vertices = new GLfloat[vertex_list.size() * 3]; // create vertex array
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertex_list.size() * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

  GLuint colours_vbo = 0;
  glGenBuffers(1, &colours_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertex_color.size() * sizeof(GLfloat), vertex_color.data(), GL_STATIC_DRAW);

  // delete VBO when program terminated
  // glDeleteBuffers(1, &vboId);
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  f->glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  f->glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
  f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  f->glEnableVertexAttribArray(0);
  f->glEnableVertexAttribArray(1);
}

void GLWidget::resizeGL(int w, int h) {
  m_width = w;
  m_height = h;
  setViewingVolume(w, h, m_zoom_factor);
  Tree->invalidate_draw();
  m_proj.setToIdentity();
  m_proj.ortho(Tree->get_left(), Tree->get_right(), Tree->get_bottom(), Tree->get_top(), -1, 1);
}

void GLWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);

  m_program->bind();
  glBindVertexArray(vao);
  glDrawArrays(GL_LINE_STRIP, 0, vertex_list.size() / 5.0);
  m_program->release();
  // Draw the grid
  // if (Tree) {
  //   Tree->forEachNode(Tree->m_rootNode, std::bind(&GLWidget::drawTreeNode, this, std::placeholders::_1));
  // }

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

void GLWidget::drawTreeNode(Node *pNode) {
  if (!pNode) {
    return;
  }

  if (!pNode->m_draw) {
    return;
  }

  pNode->m_draw = false;

  glColor3f(1, 0, 0);
  glBegin(GL_LINE_STRIP);
  glVertex3f(pNode->centre_x() - (pNode->x_dsp()), pNode->centre_y() + (pNode->y_dsp()), 0);
  glVertex3f(pNode->centre_x() - (pNode->x_dsp()), pNode->centre_y() - (pNode->y_dsp()), 0);
  glVertex3f(pNode->centre_x() + (pNode->x_dsp()), pNode->centre_y() - (pNode->y_dsp()), 0);
  glVertex3f(pNode->centre_x() + (pNode->x_dsp()), pNode->centre_y() + (pNode->y_dsp()), 0);
  glVertex3f(pNode->centre_x() - (pNode->x_dsp()), pNode->centre_y() + (pNode->y_dsp()), 0);
  glEnd();

  // glColor3f(1, 1, 1);
  // if (pNode->depth < 5) {
  //   renderText(pNode->centre_x(), pNode->centre_y(), 0, QString::number(pNode->id));
  // }

  // Node Count label
  TreeChanged(QString::fromStdString(std::to_string((Tree->getNodeCount()))));
}

void GLWidget::generate_vbo(const Node *pNode, std::vector<GLfloat> &list) {
  if (!pNode) {
    return;
  }

  list.push_back(pNode->centre_x() - pNode->x_dsp());
  list.push_back(pNode->centre_y() + pNode->y_dsp()); // list.push_back(0);
  list.push_back(pNode->centre_x() - pNode->x_dsp());
  list.push_back(pNode->centre_y() - pNode->y_dsp()); // list.push_back(0);
  list.push_back(pNode->centre_x() + pNode->x_dsp());
  list.push_back(pNode->centre_y() - pNode->y_dsp()); // list.push_back(0);
  list.push_back(pNode->centre_x() + pNode->x_dsp());
  list.push_back(pNode->centre_y() + pNode->y_dsp()); // list.push_back(0);
  list.push_back(pNode->centre_x() - pNode->x_dsp());
  list.push_back(pNode->centre_y() + pNode->y_dsp()); // list.push_back(0);

  for (auto &child : pNode->Child) {
    generate_vbo(child, list);
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
  glClearColor(0, 0, 0, 0);
  update();
}

void GLWidget::zoom(const int &p_width, const int &p_height, const double &p_factor) {

  qDebug() << "Zoom factor = " << p_factor;
  // GlobalConst::BoundingBoxLeft	*=	p_factor;
  // GlobalConst::BoundingBoxRight	*=	p_factor;
  // GlobalConst::BoundingBoxTop		*=	p_factor;
  // GlobalConst::BoundingBoxBottom	*= p_factor;

  /* A routine for setting the projection matrix. May be called from a resize
     event handler in a typical application. Takes integer width and height
     dimensions of the drawing area. Creates a projection matrix with correct
     aspect ratio and zoom factor. */

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(50.0 * p_factor, (float)p_width / (float)p_height, 1, -1);
  /* ...Where 'zNear' and 'zFar' are up to you to fill in. */
  update();
}

void GLWidget::zoomOut() {
  // Tree->updateTreeBoundary(
  // Tree->get_left()	*	1.33f,
  // Tree->get_right()	*	1.33f,
  // Tree->get_bottom()	*	1.33f,
  // Tree->get_top()		* 1.33f );

  // setViewingVolume();
}

void GLWidget::setViewingVolume(const int &p_x, const int &p_y, const double &p_zoom_factor) {

  glViewport(0, 0, p_x, p_y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluOrtho2D(Tree->get_left() * p_zoom_factor, Tree->get_right() * p_zoom_factor, Tree->get_bottom() * p_zoom_factor,
             Tree->get_top() * p_zoom_factor);

  glMatrixMode(GL_MODELVIEW);
}
