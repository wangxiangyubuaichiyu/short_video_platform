#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QVector>
#include "vilist.h"
#include "AVPlay.h"

class MyListWidget : public QScrollArea
{
    Q_OBJECT

public:
    MyListWidget(QWidget *parent = nullptr);
    ~MyListWidget();

protected:
    void resizeEvent(QResizeEvent *event) override;   // ��дresizeEvent�����ڳߴ�ı��¼�
    void updateContainerSize();                       // ����������С����Ӧ����

signals:
    void getList(ViList*& list);                      // �����ź��Ի�ȡ����
    void needList();                                  // ����Ҫ�����б�����ʱ�����ź�
    void needopen(VNode*& t);                         // ���͵�����������������

private:
    void addItemWidget(QWidget *item);                // ����������ӵ���С����

public slots:
    void onScrollValueChanged(int value);             // ������ֵ�仯ʱ�Ĵ���ۺ���
    void loadMoreItems();                             // ���ظ�����Ŀ���б�
    void disposehover(bool isHovering);               // ���������ͣ�¼�

private:
    QWidget *containerWidget;                         // ���ڳ����б��������
    QVector<int> columnHeights;                       // �洢ÿһ�еĵ�ǰ�߶�
    int columnCount = 4;                              // ������Ĭ��Ϊ4
    int itemSpacing = 20;                             // ��Ŀ֮��ļ��
    int itemWidth = 335;                              // ÿ����Ŀ�Ŀ��
    int len = 0;                                      // ��¼�Ѽ��ص���Ŀ����

    // ��ʱ�ò��ϣ�����Ҫ�����ͣ������Ƶ�ģ�����û�����ô������
    AVPlay* player;                                   // ������
};

#endif // MYLISTWIDGET_H
