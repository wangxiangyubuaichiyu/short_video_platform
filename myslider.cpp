#include "myslider.h"

#include <QMouseEvent>
#include <QStyle>

Myslider::Myslider(QWidget *parent): QSlider(parent)
{
    // 设置范围和步长
    setMinimum(0);
    setMaximum(100);
    setSingleStep(1); // 设置单步大小
    setPageStep(0);   // 禁用PageStep
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
