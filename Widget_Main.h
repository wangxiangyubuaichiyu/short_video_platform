#ifndef WIDGET_MAIN_H
#define WIDGET_MAIN_H

#include <QWidget>
#include <CVideo.h>
#include "MyThread.h"
QT_BEGIN_NAMESPACE
namespace Ui { class Widget_Main; }
QT_END_NAMESPACE

class Widget_Main : public QWidget
{
    Q_OBJECT

public:
    Widget_Main(QWidget *parent = nullptr);
    ~Widget_Main();

private slots:
    void SLT_show(QImage img);

private:
    Ui::Widget_Main *ui;
    MyThread* th;
};
#endif // WIDGET_MAIN_H
