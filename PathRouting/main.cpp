#include "AIXM_file_parser.h"
#include "GLWidget.h"
#include "QuadTree.h"
#include "pathrouting.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  PathRouting w;
  w.show();
  return a.exec();
}
