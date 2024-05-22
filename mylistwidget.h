#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QVector>

class MyListWidget : public QScrollArea
{
    Q_OBJECT

public:
    MyListWidget(QWidget *parent = nullptr);
    ~MyListWidget();

private:
    void addItemWidget(QWidget *item);     // 添加自定义 QWidget 函数
private slots:
    void onScrollValueChanged(int value);  // 滚动条数值变化槽函数
    void loadMoreItems();                  // 加载更多自定义 QWidget 槽函数

private:
    QVBoxLayout *mainLayout;              // 主布局，用于垂直排列所有行
    QGridLayout *currentRowLayout;        // 当前行的布局
    int currentColumn;                    // 当前列号
    QVector<int> columnHeights;           // 列高度列表
    int columnCount = 4;                  // 列数
    int itemSpacing = 20;                 // 项目间距
};

#endif // MYLISTWIDGET_H
