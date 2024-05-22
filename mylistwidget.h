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
    void addItemWidget(QWidget *item);     // ����Զ��� QWidget ����
private slots:
    void onScrollValueChanged(int value);  // ��������ֵ�仯�ۺ���
    void loadMoreItems();                  // ���ظ����Զ��� QWidget �ۺ���

private:
    QVBoxLayout *mainLayout;              // �����֣����ڴ�ֱ����������
    QGridLayout *currentRowLayout;        // ��ǰ�еĲ���
    int currentColumn;                    // ��ǰ�к�
    QVector<int> columnHeights;           // �и߶��б�
    int columnCount = 4;                  // ����
    int itemSpacing = 20;                 // ��Ŀ���
};

#endif // MYLISTWIDGET_H
