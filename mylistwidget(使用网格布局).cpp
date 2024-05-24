#include "mylistwidget.h"
#include <QScrollBar>
#include "json.h"
MyListWidget::MyListWidget(QWidget *parent) : QScrollArea(parent), currentColumn(0),len(0)
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
        //emit needList(); (��ʱû�з�װtcpЭ�飬����)  //���������ȡ������Ƶ�����ź�
        //-----------------�����Է�4��url------------------
        //ViList* header=NULL;
        //emit getList(header);
        //for(int i=13;i<=16;i++)
        //{
        //    header->push_back(new VNode);
        //}
        //------------------------------------------------
        loadMoreItems();                             // �������ײ�ʱ���ظ�����Ŀ
    }
}

void MyListWidget::loadMoreItems()
{
    //�����źŸ��ۣ����������е������ȡ��
    ViList* header=NULL;
    emit getList(header);
    int l=(header->getNum()-len)/4;
    VNode* t=header->check(len);
    while(l--)
    {
        for (int i = 0; i < columnCount; ++i)                               // ÿ�м����ĸ���Ŀ
        {
            QLabel *label = new QLabel();
            label->setAlignment(Qt::AlignHCenter|Qt::AlignTop);             //ˮƽ���У���ֱ����
            label->setPixmap(t->FirstImage);                                // ���� QLabel �� pixmap ����
            label->setStyleSheet("background-color: white; border-radius: 20px;");
            addItemWidget(label);
            t=t->next;
        }
    }
    len=header->getNum();
}
