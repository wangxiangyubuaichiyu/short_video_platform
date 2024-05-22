#ifndef WIDGET_MAIN_H
#define WIDGET_MAIN_H

#include <QLabel>
#include <QPoint>
#include <QTimer>
#include <QWidget>
#include "AVPlay.h"
#include "switchcontrol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Widget_Main : public QWidget
{
    Q_OBJECT

public:
    Widget_Main(QWidget *parent = nullptr);
    ~Widget_Main();
    QPixmap roundImage(const QPixmap &src, int radius);// �������ڻ���Բ�ǵ�ͼƬ

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void SLT_show(QImage img);                  //չʾ���ؼ���
    void slot_PlayerStateChanged(int state);    //״̬�л�
    void slot_getTotalTime(qint64 uSec);        //��ȡ��Ƶȫ��ʱ��
    void slot_TimerTimeOut();                   //��ȡ��ǰ��Ƶʱ�䶨ʱ��
    void slot_videoSliderValueChanged(int);     //��Ƶ����ͨ���ֶ�����ı�
    void on_btn_close_clicked();                //�رմ���
    void on_btn_big_clicked();                  //���󴰿�->�ص�����λ��
    void on_btn_small_clicked();                //��С����
    void on_btn_home_clicked();                 //��ҳ
    void on_btn_user_clicked();                 //����ҳ
    void on_btn_play_clicked();                 //�û�ҳ
    void on_btn_open_clicked();                 //��������ͣ��ť

    void on_btn_set_clicked();

private:
    Ui::MainWindow *ui;                 //ui����
    AVPlay* m_play;                     //������
    QPoint m_offset;                    //��¼ָ��������
    bool m_dragging = false;            //�������Ƿ��ڱ��λ���ƶ�����
    bool inSpecifiedArea = false;       //����Ƿ���ָ��������
    SwitchControl* m_swtc;              //������ť
    int isStop;                         //��ͣ״̬
    QTimer m_timer;                     //��ʱ��
};
#endif // WIDGET_MAIN_H
