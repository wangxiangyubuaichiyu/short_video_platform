#ifndef WIDGET_MAIN_H
#define WIDGET_MAIN_H

#include <QWidget>
#include "AVPlay.h"

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
    void SLT_show(QImage img);   //展示到控件上

private:
    Ui::Widget_Main *ui;
    AVPlay* m_play;
};
#endif // WIDGET_MAIN_H
