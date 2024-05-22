#include "mylistwidget.h"
#include <QScrollBar>

MyListWidget::MyListWidget(QWidget *parent) : QScrollArea(parent), currentColumn(0)
{
    setWidgetResizable(true);                           // ȷ�� QScrollArea ���Ե�����С����Ӧ����

    QWidget *containerWidget = new QWidget(this);       // ����һ������С����
    mainLayout = new QVBoxLayout(containerWidget);      // ��ʼ��������
    mainLayout->setSpacing(itemSpacing);                // ���������ֵļ��
    mainLayout->setAlignment(Qt::AlignTop);             // ȷ�����ݶ�����
    containerWidget->setLayout(mainLayout);
    setWidget(containerWidget);

    // ���ع�����
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // ����͸������
    setStyleSheet("QScrollArea { background: transparent; border: none; }");
    containerWidget->setStyleSheet("background: transparent;");

    // ���ӹ������źŵ��ۺ���
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &MyListWidget::onScrollValueChanged);

    // ��ʼ���и߶��б�
    columnHeights.resize(columnCount, 0);

    // ��ӳ�ʼ��Ŀ--------->δ����Ҫ�ĵ�
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
        currentRowLayout->setHorizontalSpacing(itemSpacing); // ������Ŀ֮���ˮƽ���
        currentRowLayout->setVerticalSpacing(itemSpacing);   // ������Ŀ֮��Ĵ�ֱ���
        mainLayout->addLayout(currentRowLayout);
    }

    currentRowLayout->addWidget(item, 0, currentColumn, Qt::AlignTop);
    currentColumn = (currentColumn + 1) % columnCount;
}

void MyListWidget::onScrollValueChanged(int value)
{
    if (value == verticalScrollBar()->maximum())     // ���������Ƿ�������ײ�
    {
        loadMoreItems();                             // �������ײ�ʱ���ظ�����Ŀ
    }
}

void MyListWidget::loadMoreItems()
{
    for (int i = 0; i < columnCount; ++i)                                                           // ÿ�м����ĸ���Ŀ
    {
        QLabel *label = new QLabel(QString("Item %1").arg(mainLayout->count() * columnCount + i));  // ����һ�������ı��ı�ǩ
        label->setFixedSize(335, 345);                                                              // ����QLabel�Ĵ�С
        label->setStyleSheet("background-color: lightgreen; border-radius: 25px;");                 // ���ñ�ǩ����ʽ
        addItemWidget(label);                                                                       // �ڲ�������ӱ�ǩ
    }
}
