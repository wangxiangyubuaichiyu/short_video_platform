#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H

#include <QObject>
#include <QDebug>

// 前置声明 VNode 类，以便在头文件中引用而不引起循环包含
class VNode;

class MouseEvent : public QObject
{
    Q_OBJECT
public:
    explicit MouseEvent(VNode *node = nullptr, QObject *parent = nullptr);  // 修改构造函数以接受 VNode* 参数

signals:
    void hovered(bool isHovering);
    void mouseClicked(VNode *node);  // 修改 mouseClicked 信号以接受 VNode* 参数

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    VNode *m_node;  // 存储 VNode 指针
};

#endif // MOUSEEVENT_H
