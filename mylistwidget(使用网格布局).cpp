#include "mylistwidget.h"
#include <QScrollBar>
#include "json.h"
MyListWidget::MyListWidget(QWidget *parent) : QScrollArea(parent), currentColumn(0),len(0)
{
    setWidgetResizable(true);                           // 确保 QScrollArea 可以调整大小以适应内容

    QWidget *containerWidget = new QWidget(this);       // 创建一个容器小部件
    mainLayout = new QVBoxLayout(containerWidget);      // 初始化主布局
    mainLayout->setSpacing(itemSpacing);                // 设置主布局的间距
    mainLayout->setAlignment(Qt::AlignTop);             // 确保内容顶对齐
    containerWidget->setLayout(mainLayout);
    setWidget(containerWidget);

    // 隐藏滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置透明背景
    setStyleSheet("QScrollArea { background: transparent; border: none; }");
    containerWidget->setStyleSheet("background: transparent;");

    // 连接滚动条信号到槽函数
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &MyListWidget::onScrollValueChanged);

    // 初始化列高度列表
    columnHeights.resize(columnCount, 0);

}

MyListWidget::~MyListWidget()
{
}

void MyListWidget::addItemWidget(QWidget *item)
{
    if (currentColumn == 0)
    {
        currentRowLayout = new QGridLayout();
        currentRowLayout->setHorizontalSpacing(itemSpacing); // 设置项目之间的水平间距
        currentRowLayout->setVerticalSpacing(itemSpacing);   // 设置项目之间的垂直间距
        mainLayout->addLayout(currentRowLayout);
    }

    currentRowLayout->addWidget(item, 0, currentColumn, Qt::AlignTop);
    currentColumn = (currentColumn + 1) % columnCount;
}

void MyListWidget::onScrollValueChanged(int value)
{
    if (value == verticalScrollBar()->maximum())     // 检查滚动条是否滚动到底部
    {
        //emit needList(); (暂时没有封装tcp协议，保留)  //发送请求获取更多视频链接信号
        //-----------------测试性放4个url------------------
        //ViList* header=NULL;
        //emit getList(header);
        //for(int i=13;i<=16;i++)
        //{
        //    header->push_back(new VNode);
        //}
        //------------------------------------------------
        loadMoreItems();                             // 滚动到底部时加载更多项目
    }
}

void MyListWidget::loadMoreItems()
{
    //采用信号跟槽，将主窗口中的链表获取到
    ViList* header=NULL;
    emit getList(header);
    int l=(header->getNum()-len)/4;
    VNode* t=header->check(len);
    while(l--)
    {
        for (int i = 0; i < columnCount; ++i)                               // 每行加载四个项目
        {
            QLabel *label = new QLabel();
            label->setAlignment(Qt::AlignHCenter|Qt::AlignTop);             //水平居中，垂直向上
            label->setPixmap(t->FirstImage);                                // 设置 QLabel 的 pixmap 属性
            label->setStyleSheet("background-color: white; border-radius: 20px;");
            addItemWidget(label);
            t=t->next;
        }
    }
    len=header->getNum();
}
