#include "myslider.h"

#include <QMouseEvent>
#include <QStyle>

Myslider::Myslider(QWidget *parent): QSlider(parent)
{
    // ���÷�Χ�Ͳ���
    setMinimum(0);
    setMaximum(100);
    setSingleStep(1); // ���õ�����С
    setPageStep(0);   // ����PageStep
}

Myslider::~Myslider()
{

}
void Myslider::mousePressEvent(QMouseEvent *ev)
{
    int value = QStyle::sliderValueFromPosition(minimum(), maximum(), ev->pos().x(), width());
    setValue(value);
    emit SIG_valueChanged(value);
}
