#include "mylistwidget.h"
#include <QScrollBar>

MyListWidget::MyListWidget(QWidget *parent) : QScrollArea(parent), currentColumn(0)
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

    // 添加初始项目--------->未来需要改的
    const int initialItemCountPerColumn = 2;
    for (int i = 0; i < columnCount * initialItemCountPerColumn; ++i)
    {
        loadMoreItems();
    }
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
        loadMoreItems();                             // 滚动到底部时加载更多项目
    }
}

void MyListWidget::loadMoreItems()
{
    for (int i = 0; i < columnCount; ++i)                                                           // 每行加载四个项目
    {
        QLabel *label = new QLabel(QString("Item %1").arg(mainLayout->count() * columnCount + i));  // 创建一个带有文本的标签
        label->setFixedSize(335, 345);                                                              // 设置QLabel的大小
        label->setStyleSheet("background-color: lightgreen; border-radius: 25px;");                 // 设置标签的样式
        addItemWidget(label);                                                                       // 在布局中添加标签
    }
}
