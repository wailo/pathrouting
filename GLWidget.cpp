#include "GLWidget.h"
#include <QtDebug>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <array>

GLWidget::GLWidget(QWidget *parent)
    : m_zoom_factor(1.0), QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
      Tree(new QuadTree(0, 2048, 0, 2048)) // Initialize a new quadtree
{
  setMouseTracking(true);


  // Read Data
  // data.read_AIXM_file( "..\\Airport_data\\Chicago_Airspace_CRS84.xml");
  bool OK = false;

  OK = data.read_AIXM_file("Airport_data//Chicago O'Hare Aprons_CRS84.xml") &&
       data.read_AIXM_file("Airport_data//Chicago O'Hare Taxiways_CRS84.xml") &&
       data.read_AIXM_file("Airport_data//Chicago O'Hare Runways_CRS84.xml") &&
       data.read_AIXM_file("Airport_data//Chicago O'Hare VerticalStructures_CRS84.xml");
  // data.read_AIXM_file( "..//Airport_data//Chicago O'Hare Taxiways.xml");
  // data.read_AIXM_file( "..//Airport_data//Chicago O'Hare Runways.xml");

  if (!OK) {
    return;
  }
  data.process_boundaries(*Tree);

  const GLuint NumVertices = Tree->getNodeCount() * 4;
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

  //	setAutoBufferSwap( TRUE );
  //	glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  //	glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_BLEND);
  //	glEnable(GL_POLYGON_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0, 0, 0, 0);
  //
}

void GLWidget::resizeGL(int w, int h) {
  m_width = w;
  m_height = h;
  setViewingVolume(w, h, m_zoom_factor);
}

void GLWidget::paintGL() {

  // Draw the grid
  if (Tree) {
    Tree->forEachNode(Tree->m_rootNode, std::bind(&GLWidget::drawTreeNode, this, std::placeholders::_1));
  }

  for (std::vector<AIXM_file_parser::GMLObject *>::const_iterator itr = data.Objects.begin(); itr != data.Objects.end();
       ++itr) {

    if ((*itr)->m_AIXM_object_type.compare("GuidanceLine") == 0) {
      glColor3f(0, 1, 0);
    } else if ((*itr)->m_AIXM_object_type.compare("TaxiwayElement") == 0) {
      glColor3f(1, 1, 0);
      // continue;
    } else if ((*itr)->m_AIXM_object_type.compare("RunwayElement") == 0) {
      glColor3f(0, 0, 1);
      // continue;
    } else {
      // continue;
    }

    glPointSize(3);
    glBegin(GL_LINE_STRIP);
    for (unsigned int j = 0; j < (*itr)->m_Coordinates.size() - 1; ++j) {
      // qDebug() << data.Objects.at( i )->Lons.at( j )-x+x_dist;
      glVertex3f((*itr)->m_Coordinates.at(j).m_Lon, (*itr)->m_Coordinates.at(j).m_Lat, 0);
    }
    glEnd();
  }

  // Draw the shortest path
  glColor4f(0, 1, 0, 0.5);
  for (std::vector<Node *>::iterator itr = Tree->camefromSet.begin(); itr != Tree->camefromSet.end(); ++itr) {
    glBegin(GL_POLYGON);
    glVertex3f((*itr)->centre_x() - ((*itr)->x_dsp()), (*itr)->centre_y() + ((*itr)->y_dsp()), 0);
    glVertex3f((*itr)->centre_x() - ((*itr)->x_dsp()), (*itr)->centre_y() - ((*itr)->y_dsp()), 0);
    glVertex3f((*itr)->centre_x() + ((*itr)->x_dsp()), (*itr)->centre_y() - ((*itr)->y_dsp()), 0);
    glVertex3f((*itr)->centre_x() + ((*itr)->x_dsp()), (*itr)->centre_y() + ((*itr)->y_dsp()), 0);
    glEnd();
  }

  Tree->camefromSet.clear();
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

  glColor3f(1, 1, 1);
  if (pNode->depth < 5) {
    renderText(pNode->centre_x(), pNode->centre_y(), 0, QString::number(pNode->id));
  }

  // Node Count label
  TreeChanged(QString::fromStdString(std::to_string((Tree->getNodeCount()))));
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  double world_x, world_y, world_z;
  double model_view[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

  double projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  gluUnProject(event->x(), viewport[3] - event->y(), 0.0, model_view, projection, viewport, &world_x, &world_y,
               &world_z);

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
  updateGL();
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
  updateGL();
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
  updateGL();
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
  double canvas_width = fabs(Tree->get_left() - Tree->get_right());
  double canvas_height = fabs(Tree->get_top() - Tree->get_bottom());
  double aspect_ratio = (double)canvas_width / (double)canvas_height;

  glViewport(0, 0, p_x, p_y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (canvas_width > canvas_height) {
    // gluOrtho2D((p_x+Tree->get_left())*aspect_ratio,(p_x+Tree->get_right())*aspect_ratio,p_y+Tree->get_bottom(),p_y+Tree->get_top());
  } else {
    // gluOrtho2D(p_x+Tree->get_left(),p_x+Tree->get_right(),(p_y+Tree->get_bottom())*aspect_ratio,(p_y+Tree->get_top())*aspect_ratio);
  }

  gluOrtho2D(Tree->get_left() * p_zoom_factor, Tree->get_right() * p_zoom_factor, Tree->get_bottom() * p_zoom_factor,
             Tree->get_top() * p_zoom_factor);
  // updateZoomFactors();
  glMatrixMode(GL_MODELVIEW);
}