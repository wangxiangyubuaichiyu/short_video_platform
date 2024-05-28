#include "mylistwidget.h"
#include "mouseevent.h"
#include <QScrollBar>
#include "json.h"
#include <QLabel>
#include <QDebug>

MyListWidget::MyListWidget(QWidget *parent) : QScrollArea(parent), len(0)
{
    player=new AVPlay;

    setWidgetResizable(true);                                       // �������������������Զ�������С
    containerWidget = new QWidget(this);                            // �ڹ������ڴ�������С����
    containerWidget->setStyleSheet("background: transparent;");     // ������������͸��
    setWidget(containerWidget);                                     // ����������Ϊ����������ҪС����

    // ���ù��������ԣ����ع��������������Կ�ͨ�����ֹ���
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("QScrollArea { background: transparent; border: none; }");  // ���ù�����͸���ޱ߿�

    // ���ӹ�����ֵ�仯���źŵ���Ӧ�Ĳۺ���
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &MyListWidget::onScrollValueChanged);

    // ��ʼ���и߶����������ó�ʼֵΪ0
    columnHeights.resize(columnCount, 0);
}

MyListWidget::~MyListWidget()
{
    delete player;
}

// �����б��еĺ���
void MyListWidget::addItemWidget(QWidget *item)
{
    int minHeightIndex = 0;                         // �����ҵ�����е�����
    int minHeight = columnHeights[0];               // ��ʼ��Ϊ��һ�еĸ߶�
    for (int i = 1; i < columnCount; ++i)
    {
        if (columnHeights[i] < minHeight)           // �ҵ����̵���
        {
            minHeight = columnHeights[i];
            minHeightIndex = i;
        }
    }

    // ����Ҫ��ӵ�С������λ��
    int x = minHeightIndex * (itemWidth + itemSpacing);             // ˮƽλ��
    int y = columnHeights[minHeightIndex];                          // ��ֱλ�ã����еĵ�ǰ�߶�
    item->setParent(containerWidget);                               // ����С�����ĸ�����
    item->setGeometry(x, y, itemWidth, item->sizeHint().height());  // ����С�����ļ�����״
    item->show();                                                   // ��ʾС����

    // �����и߶�
    columnHeights[minHeightIndex] += item->sizeHint().height() + itemSpacing;
    updateContainerSize();                                          // ����������С
}

// ������ֵ�仯ʱ���õĲۺ���
void MyListWidget::onScrollValueChanged(int value)
{
    if (value == verticalScrollBar()->maximum())    // �������������ײ�
    {
        //emit needList(); (��ʱû�з�װtcpЭ�飬����)  //���������ȡ������Ƶ�����ź�
        //-----------------�����Է�4��url------------------
        //ViList* header=NULL;
        //emit getList(header);
        //for(int i=21;i<=24;i++)
        //{
        //    header->push_back(new VNode);
        //}
        //------------------------------------------------
        loadMoreItems();                            // ���ظ�����Ŀ
    }
}

// ���ظ�����Ŀ�ĺ���
void MyListWidget::loadMoreItems()
{
    ViList* header = NULL;                                  // ����ͷָ���ʼ��Ϊ��
    emit getList(header);                                   // �����źţ������ȡ����
    int l = (header->getNum() - len) / 4;                   // ������Ҫ��ӵ�����
    VNode* t = header->check(len);                          // ��ȡ��ǰλ�õĽڵ�
    while (l--)
    {
        for (int i = 0; i < columnCount; ++i)
        {
            // ����һ������������ǩ��QWidget����
            QWidget *container = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(container);               // ʹ�ô�ֱ����
            layout->setContentsMargins(0, 0, 0, 0);                         // ���ò��ֵ���߾�Ϊ0
            layout->setSpacing(0);                                          // ���ò����ڸ��ؼ�֮��ļ��Ϊ0

            // ����ͼƬ��ǩ
            QLabel *imageLabel = new QLabel();
            imageLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);         // ���ÿ����ϽǶ���
            QSize newSize(itemWidth, t->FirstImage.size().height());        // �����µĳߴ�
            t->FirstImage = t->FirstImage.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // ����ͼƬ
            imageLabel->setPixmap(t->FirstImage);
            imageLabel->setFixedSize(335, t->FirstImage.size().height());

            // �����ı���ǩ
            QLabel *textLabel = new QLabel(t->name);                                     // ������ʾ���ı�
            textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);                       // ���ÿ����ϽǶ���
            textLabel->setStyleSheet("background-color: rgb(37,38,50); color: white;");  // ������ʽ

            // ����ǩ��ӵ�������
            layout->addWidget(imageLabel);
            layout->addWidget(textLabel);

            container->setLayout(layout);                                                       // ���������Ĳ���
            container->setStyleSheet("background-color: rgb(37,38,50); border-radius: 20px;");  // ����������ʽ
            container->setFixedSize(335, t->FirstImage.size().height() + 50);                   // ���ù̶���С

            // Ӧ���¼��������������ݵ�ǰ�ڵ� t
            MouseEvent *mouseevent = new MouseEvent(t);
            container->installEventFilter(mouseevent);
            container->setAttribute(Qt::WA_Hover);                                              // ������ͣ�¼�
            addItemWidget(container);                                                           // ��������ӵ���Ľ�����
            connect(mouseevent, &MouseEvent::hovered, this, &MyListWidget::disposehover);       // ���ò���
            connect(mouseevent, &MouseEvent::mouseClicked, this, [&](VNode* clickedNode){       // ����ת��������
                emit needopen(clickedNode);
            });
            t = t->next;
        }
    }
    len = header->getNum();
}


void MyListWidget::disposehover(bool isHovering)
{
    if(isHovering)              //��ͣ
    {

    }
    else                        //��ȥ
    {

    }
}

// �����ڴ�С�仯���¼�
void MyListWidget::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);    // ���û���ʵ��
    updateContainerSize();              // ����������С
}

// ����������С�ĺ���
void MyListWidget::updateContainerSize()
{
    int maxHeight = *std::max_element(columnHeights.begin(), columnHeights.end());  // �������߶�
    int requiredWidth = columnCount * (itemWidth + itemSpacing) - itemSpacing;      // ������Ҫ�Ŀ��
    containerWidget->setMinimumSize(requiredWidth, maxHeight);                      // ������������С�ߴ�
}
