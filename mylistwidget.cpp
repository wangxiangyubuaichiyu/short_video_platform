#include "mylistwidget.h"
#include <QScrollBar>
#include "json.h"
#include <QLabel>
#include <QDebug>

MyListWidget::MyListWidget(QWidget *parent) : QScrollArea(parent), len(0)
{
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
    // ĿǰΪ�գ���Ϊû����ʽ�������Դ��Ҫ�ͷ�
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
            QLabel *label = new QLabel();                               // �����µı�ǩ
            label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);       // ���ö��뷽ʽ
            QSize newSize(itemWidth, t->FirstImage.size().height());    // �����µĳߴ�
            t->FirstImage = t->FirstImage.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // ����ͼƬ
            label->setPixmap(t->FirstImage);
            label->setFixedSize(335,t->FirstImage.size().height()+50);
            //label->setText(t->name);                                    // �����ı�����
            label->setStyleSheet("background-color: rgb(37,38,50); border-radius: 20px;");
            addItemWidget(label);
            t = t->next;
        }
    }
    len = header->getNum();
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
