#ifndef VILIST_H
#define VILIST_H

#include <QObject>
#include <QPixmap>
//�������������棬�ͻ��˷��������ӣ����������
struct VNode
{
    QString url;            //����
    int Id;                 //�������
    QString name;           //��Ƶ����
    QPixmap FirstImage;     //��һ֡����
    VNode* next;            //��һ���ڵ�
    //��չ���ݣ�δ�����
};
class ViList
{
public:
    ViList();
    ~ViList();
    //��
    void push_back(VNode* p);
    //ɾ
    void pop(int id);
    //��
    void change(int id);
    //��
    VNode*  check(int id);
    int getNum() const;

private:
    VNode* header;
    VNode* end;
    int   num;
};

#endif // VILIST_H
