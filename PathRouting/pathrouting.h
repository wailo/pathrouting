#ifndef PATHROUTING_H
#define PATHROUTING_H

#include <QtWidgets/QWidget>
#include "ui_pathrouting.h"

class PathRouting : public QWidget
{
	Q_OBJECT

public:
	PathRouting(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~PathRouting();

private:
	Ui::PathRoutingClass ui;
	
};

#endif // PATHROUTING_H
