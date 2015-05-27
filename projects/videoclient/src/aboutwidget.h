#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QWidget>

#include "ui_aboutwidget.h"

class AboutWidget : public QWidget
{
  Q_OBJECT

public:
  AboutWidget(QWidget *parent = nullptr, Qt::WindowFlags f = 0);

private:
  Ui::AboutWidget ui;
};

#endif