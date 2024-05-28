#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QSlider>
#include <QObject>

class Myslider : public QSlider
{
    Q_OBJECT
public:
    Myslider(QWidget *parent = 0);
    ~Myslider();
    void mousePressEvent(QMouseEvent *ev)override;

signals:
    void SIG_valueChanged(int);
};

#endif // MYSLIDER_H
