#include "pathrouting.h"
#include "GLWidget.h"
#include <QtWidgets/QApplication>
#include "QuadTree.h"
#include "AIXM_file_parser.h"


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  PathRouting w;
  w.show();
  return a.exec();
}
