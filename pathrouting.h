#ifndef PATHROUTING_H
#define PATHROUTING_H

#include "ui_pathrouting.h"
#include <QtWidgets/QWidget>

class PathRouting : public QWidget {
  Q_OBJECT

public:
  PathRouting(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~PathRouting();

private:
  Ui::PathRoutingClass ui;
};

#endif // PATHROUTING_H
