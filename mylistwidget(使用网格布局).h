#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QVector>
#include "vilist.h"

class MyListWidget : public QScrollArea
{
    Q_OBJECT

public:
    MyListWidget(QWidget *parent = nullptr);
    ~MyListWidget();
signals:
    void getList(ViList*& list);           //��ȡ����
    void needList();                       //��������ȫ����ʾ����Ҫ���������������
private:
    void addItemWidget(QWidget *item);     // ����Զ��� QWidget ����
public slots:
    void onScrollValueChanged(int value);  // ��������ֵ�仯�ۺ���
    void loadMoreItems();                  // ���ظ����Զ��� QWidget �ۺ���

private:
    QVBoxLayout *mainLayout;              // �����֣����ڴ�ֱ����������
    QGridLayout *currentRowLayout;        // ��ǰ�еĲ���
    int currentColumn;                    // ��ǰ�к�
    QVector<int> columnHeights;           // �и߶��б�
    int columnCount = 4;                  // ����
    int itemSpacing = 20;                 // ��Ŀ���
    int len=0;                            // ���ж��ٸ�label
};

#endif // MYLISTWIDGET_H
