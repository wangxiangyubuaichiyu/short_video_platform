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
    void resizeEvent(QResizeEvent *event) override;   // 重写resizeEvent处理窗口尺寸改变事件
    void updateContainerSize();                       // 更新容器大小以适应内容

signals:
    void getList(ViList*& list);                      // 发出信号以获取链表
    void needList();                                  // 当需要更多列表数据时发出信号
    void needopen(VNode*& t);                         // 发送到播放器上用来播放

private:
    void addItemWidget(QWidget *item);                // 向容器中添加单个小部件

public slots:
    void onScrollValueChanged(int value);             // 滚动条值变化时的处理槽函数
    void loadMoreItems();                             // 加载更多项目到列表
    void disposehover(bool isHovering);               // 处理鼠标悬停事件

private:
    QWidget *containerWidget;                         // 用于承载列表项的容器
    QVector<int> columnHeights;                       // 存储每一列的当前高度
    int columnCount = 4;                              // 列数，默认为4
    int itemSpacing = 20;                             // 项目之间的间隔
    int itemWidth = 335;                              // 每个项目的宽度
    int len = 0;                                      // 记录已加载的项目数量

    // 暂时用不上，这是要鼠标悬停播放视频的，但是没想好怎么播合适
    AVPlay* player;                                   // 播放器
};

#endif // MYLISTWIDGET_H
