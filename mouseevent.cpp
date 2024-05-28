#include "mouseevent.h"
#include "vilist.h"

#include <QEvent>

MouseEvent::MouseEvent(VNode *node, QObject *parent)
    : QObject(parent), m_node(node)  // 初始化 m_node
{

}

bool MouseEvent::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::HoverEnter:
        //qDebug() << "Mouse hover over";
        emit hovered(true);  // 发射悬停进入信号
        break;
    case QEvent::HoverLeave:
        //qDebug() << "Mouse hover leave";
        emit hovered(false);  // 发射悬停离开信号
        break;
    case QEvent::MouseButtonPress:
        //qDebug() << "Mouse button pressed";
        emit mouseClicked(m_node);  // 发射鼠标点击信号，并传递存储的 VNode 指针
        break;
    case QEvent::MouseButtonRelease:
        //qDebug() << "Mouse button released";
        break;
    default:
        break;
    }
    return QObject::eventFilter(obj, event);  // 确保其他事件正常处理
}
