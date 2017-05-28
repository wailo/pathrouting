
#include "pathrouting.h"
#include "GLWidget.h"
#include <QtWidgets/QApplication>
#include "Quadtree.h"
#include "AIXM_file_parser.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	PathRouting w;
	 //GLDrawer p( &w , 0 );

	

	w.show();
	return a.exec();
}
