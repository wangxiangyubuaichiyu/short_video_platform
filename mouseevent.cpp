#include "mouseevent.h"
#include "vilist.h"

#include <QEvent>

MouseEvent::MouseEvent(VNode *node, QObject *parent)
    : QObject(parent), m_node(node)  // ��ʼ�� m_node
{

}

bool MouseEvent::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::HoverEnter:
        //qDebug() << "Mouse hover over";
        emit hovered(true);  // ������ͣ�����ź�
        break;
    case QEvent::HoverLeave:
        //qDebug() << "Mouse hover leave";
        emit hovered(false);  // ������ͣ�뿪�ź�
        break;
    case QEvent::MouseButtonPress:
        //qDebug() << "Mouse button pressed";
        emit mouseClicked(m_node);  // ����������źţ������ݴ洢�� VNode ָ��
        break;
    case QEvent::MouseButtonRelease:
        //qDebug() << "Mouse button released";
        break;
    default:
        break;
    }
    return QObject::eventFilter(obj, event);  // ȷ�������¼���������
}
