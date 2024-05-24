#ifndef VILIST_H
#define VILIST_H

#include <QObject>
#include <QPixmap>
//此链表用来保存，客户端发来的连接，和相关数据
struct VNode
{
    QString url;            //链接
    int Id;                 //传入序号
    QString name;           //视频标题
    QPixmap FirstImage;     //第一帧数据
    VNode* next;            //下一个节点
    //扩展数据，未来添加
};
class ViList
{
public:
    ViList();
    ~ViList();
    //增
    void push_back(VNode* p);
    //删
    void pop(int id);
    //改
    void change(int id);
    //查
    VNode*  check(int id);
    int getNum() const;

private:
    VNode* header;
    VNode* end;
    int   num;
};

#endif // VILIST_H
