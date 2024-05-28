#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H

#include <QObject>
#include <QDebug>

// ǰ������ VNode �࣬�Ա���ͷ�ļ������ö�������ѭ������
class VNode;

class MouseEvent : public QObject
{
    Q_OBJECT
public:
    explicit MouseEvent(VNode *node = nullptr, QObject *parent = nullptr);  // �޸Ĺ��캯���Խ��� VNode* ����

signals:
    void hovered(bool isHovering);
    void mouseClicked(VNode *node);  // �޸� mouseClicked �ź��Խ��� VNode* ����

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    VNode *m_node;  // �洢 VNode ָ��
};

#endif // MOUSEEVENT_H
