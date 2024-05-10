#ifndef WIDGET_MAIN_H
#define WIDGET_MAIN_H

#include <QPoint>
#include <QWidget>
#include "AVPlay.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Widget_Main : public QWidget
{
    Q_OBJECT

public:
    Widget_Main(QWidget *parent = nullptr);
    ~Widget_Main();
protected:
    //�϶����ڿ��Խ�����ק
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private slots:
    void SLT_show(QImage img);   //չʾ���ؼ���
    void on_btn_close_clicked(); //�رմ���
    void on_btn_big_clicked();   //���󴰿�->�ص�����λ��
    void on_btn_small_clicked(); //��С����

    void on_btn_home_clicked(); //��ҳ
    void on_btn_user_clicked(); //����ҳ
    void on_btn_play_clicked(); //�û�ҳ
    void on_btn_open_clicked(); //��������ͣ��ť

private:
    Ui::MainWindow *ui;  //ui����
    AVPlay* m_play;      //������
    QPoint m_offset;     //��¼ָ��������
    bool m_dragging = false;

    //HomeWindow homeWnd;

};
#endif // WIDGET_MAIN_H
