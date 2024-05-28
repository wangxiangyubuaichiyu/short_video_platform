#include "mylistwidget.h"
#include "mouseevent.h"
#include <QScrollBar>
#include "json.h"
#include <QLabel>
#include <QDebug>

MyListWidget::MyListWidget(QWidget *parent) : QScrollArea(parent), len(0)
{
    player=new AVPlay;

    setWidgetResizable(true);                                       // 允许滚动区域根据内容自动调整大小
    containerWidget = new QWidget(this);                            // 在滚动区内创建容器小部件
    containerWidget->setStyleSheet("background: transparent;");     // 设置容器背景透明
    setWidget(containerWidget);                                     // 将容器设置为滚动区的主要小部件

    // 设置滚动条策略，隐藏滚动条，但内容仍可通过滚轮滚动
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("QScrollArea { background: transparent; border: none; }");  // 设置滚动区透明无边框

    // 连接滚动条值变化的信号到对应的槽函数
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &MyListWidget::onScrollValueChanged);

    // 初始化列高度向量，设置初始值为0
    columnHeights.resize(columnCount, 0);
}

MyListWidget::~MyListWidget()
{
    delete player;
}

// 添加项到列表中的函数
void MyListWidget::addItemWidget(QWidget *item)
{
    int minHeightIndex = 0;                         // 用于找到最短列的索引
    int minHeight = columnHeights[0];               // 初始化为第一列的高度
    for (int i = 1; i < columnCount; ++i)
    {
        if (columnHeights[i] < minHeight)           // 找到更短的列
        {
            minHeight = columnHeights[i];
            minHeightIndex = i;
        }
    }

    // 计算要添加的小部件的位置
    int x = minHeightIndex * (itemWidth + itemSpacing);             // 水平位置
    int y = columnHeights[minHeightIndex];                          // 垂直位置，即列的当前高度
    item->setParent(containerWidget);                               // 设置小部件的父容器
    item->setGeometry(x, y, itemWidth, item->sizeHint().height());  // 设置小部件的几何形状
    item->show();                                                   // 显示小部件

    // 更新列高度
    columnHeights[minHeightIndex] += item->sizeHint().height() + itemSpacing;
    updateContainerSize();                                          // 更新容器大小
}

// 滚动条值变化时调用的槽函数
void MyListWidget::onScrollValueChanged(int value)
{
    if (value == verticalScrollBar()->maximum())    // 如果滚动到了最底部
    {
        //emit needList(); (暂时没有封装tcp协议，保留)  //发送请求获取更多视频链接信号
        //-----------------测试性放4个url------------------
        //ViList* header=NULL;
        //emit getList(header);
        //for(int i=21;i<=24;i++)
        //{
        //    header->push_back(new VNode);
        //}
        //------------------------------------------------
        loadMoreItems();                            // 加载更多项目
    }
}

// 加载更多项目的函数
void MyListWidget::loadMoreItems()
{
    ViList* header = NULL;                                  // 链表头指针初始化为空
    emit getList(header);                                   // 触发信号，请求获取链表
    int l = (header->getNum() - len) / 4;                   // 计算需要添加的行数
    VNode* t = header->check(len);                          // 获取当前位置的节点
    while (l--)
    {
        for (int i = 0; i < columnCount; ++i)
        {
            // 创建一个包含两个标签的QWidget容器
            QWidget *container = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(container);               // 使用垂直布局
            layout->setContentsMargins(0, 0, 0, 0);                         // 设置布局的外边距为0
            layout->setSpacing(0);                                          // 设置布局内各控件之间的间距为0

            // 创建图片标签
            QLabel *imageLabel = new QLabel();
            imageLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);         // 设置靠左上角对齐
            QSize newSize(itemWidth, t->FirstImage.size().height());        // 计算新的尺寸
            t->FirstImage = t->FirstImage.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 缩放图片
            imageLabel->setPixmap(t->FirstImage);
            imageLabel->setFixedSize(335, t->FirstImage.size().height());

            // 创建文本标签
            QLabel *textLabel = new QLabel(t->name);                                     // 设置显示的文本
            textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);                       // 设置靠左上角对齐
            textLabel->setStyleSheet("background-color: rgb(37,38,50); color: white;");  // 设置样式

            // 将标签添加到布局中
            layout->addWidget(imageLabel);
            layout->addWidget(textLabel);

            container->setLayout(layout);                                                       // 设置容器的布局
            container->setStyleSheet("background-color: rgb(37,38,50); border-radius: 20px;");  // 设置容器样式
            container->setFixedSize(335, t->FirstImage.size().height() + 50);                   // 设置固定大小

            // 应用事件过滤器，并传递当前节点 t
            MouseEvent *mouseevent = new MouseEvent(t);
            container->installEventFilter(mouseevent);
            container->setAttribute(Qt::WA_Hover);                                              // 启用悬停事件
            addItemWidget(container);                                                           // 将容器添加到你的界面中
            connect(mouseevent, &MouseEvent::hovered, this, &MyListWidget::disposehover);       // 设置播放
            connect(mouseevent, &MouseEvent::mouseClicked, this, [&](VNode* clickedNode){       // 设置转到播放器
                emit needopen(clickedNode);
            });
            t = t->next;
        }
    }
    len = header->getNum();
}


void MyListWidget::disposehover(bool isHovering)
{
    if(isHovering)              //悬停
    {

    }
    else                        //离去
    {

    }
}

// 处理窗口大小变化的事件
void MyListWidget::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);    // 调用基类实现
    updateContainerSize();              // 更新容器大小
}

// 更新容器大小的函数
void MyListWidget::updateContainerSize()
{
    int maxHeight = *std::max_element(columnHeights.begin(), columnHeights.end());  // 计算最大高度
    int requiredWidth = columnCount * (itemWidth + itemSpacing) - itemSpacing;      // 计算需要的宽度
    containerWidget->setMinimumSize(requiredWidth, maxHeight);                      // 设置容器的最小尺寸
}
