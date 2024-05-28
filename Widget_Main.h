#ifndef WIDGET_MAIN_H
#define WIDGET_MAIN_H

#include <QLabel>
#include <QPoint>
#include <QTimer>
#include <QWidget>
#include "AVPlay.h"
#include "switchcontrol.h"
#include "vilist.h"
#include "json.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Widget_Main : public QWidget
{
    Q_OBJECT

public:
    Widget_Main(QWidget *parent = nullptr);
    ~Widget_Main();
    QPixmap roundImage(const QPixmap &src, int radius); // �������ڻ���Բ�ǵ�ͼƬ
    QImage getFirstImage(QString filePath);             //��ȡ��һ֡����
    QString getFilename(const QString &filePath);       //��ȡ��Ƶ����

protected:

    //��д�¼�
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void getListNode();                                 //�����źŸ�tcp��ȱ�ٽڵ���

private slots:
    void playMode();                                    //�����Ƿ�����
    void SLT_show(QImage img);                          //չʾ���ؼ���
    void addList(const QMap<QString, QVariant>& data);  //��ȡ����
    void sendDate(const QString& json);                 //��������
    void home2Open(VNode*& t);                          //���Ŵ�������Ƶ
    void slot_PlayerStateChanged(int state);            //״̬�л�
    void slot_getTotalTime(qint64 uSec);                //��ȡ��Ƶȫ��ʱ��
    void slot_TimerTimeOut();                           //��ȡ��ǰ��Ƶʱ�䶨ʱ��
    void slot_videoSliderValueChanged(int);             //��Ƶ����ͨ���ֶ�����ı�
    void on_btn_close_clicked();                        //�رմ���
    void on_btn_big_clicked();                          //���󴰿�->�ص�����λ��
    void on_btn_small_clicked();                        //��С����
    void on_btn_home_clicked();                         //��ҳ
    void on_btn_user_clicked();                         //����ҳ
    void on_btn_play_clicked();                         //�û�ҳ
    void on_btn_open_clicked();                         //��������ͣ��ť
    void on_btn_set_clicked();                          //���ý���
    void on_lb_on_clicked();                            //��һ����Ƶ
    void on_lb_down_clicked();                          //��һ����Ƶ

private:
    Ui::MainWindow *ui;                                 //ui����
    AVPlay* m_play;                                     //������
    QPoint m_offset;                                    //��¼ָ��������
    bool m_dragging = false;                            //�������Ƿ��ڱ��λ���ƶ�����
    bool inSpecifiedArea = false;                       //����Ƿ���ָ��������
    SwitchControl* m_swtc;                              //������ť
    QTimer m_timer;                                     //��ʱ��
    ViList* m_list;                                     //��Ƶ����б�
    VNode* m_NowNode;                                   //��ǰ�ڵ�
    Json* m_js;                                         //json������
    int num=0;                                          //��Ƶ����
    bool m_playmode=false;                              //���ű�־
};
#endif // WIDGET_MAIN_H
