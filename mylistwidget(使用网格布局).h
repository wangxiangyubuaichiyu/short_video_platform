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
    void getList(ViList*& list);           //获取链表
    void needList();                       //链表内容全部显示，需要往链表内添加数据
private:
    void addItemWidget(QWidget *item);     // 添加自定义 QWidget 函数
public slots:
    void onScrollValueChanged(int value);  // 滚动条数值变化槽函数
    void loadMoreItems();                  // 加载更多自定义 QWidget 槽函数

private:
    QVBoxLayout *mainLayout;              // 主布局，用于垂直排列所有行
    QGridLayout *currentRowLayout;        // 当前行的布局
    int currentColumn;                    // 当前列号
    QVector<int> columnHeights;           // 列高度列表
    int columnCount = 4;                  // 列数
    int itemSpacing = 20;                 // 项目间距
    int len=0;                            // 现有多少个label
};

#endif // MYLISTWIDGET_H
