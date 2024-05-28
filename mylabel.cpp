#include "mylabel.h"

#include <QMouseEvent>

Mylabel::Mylabel(QWidget* parent)
    : QLabel(parent)
{
    setMouseTracking(true);
}

void Mylabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}
