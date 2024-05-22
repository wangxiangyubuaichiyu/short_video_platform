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
    QPixmap roundImage(const QPixmap &src, int radius);// 函数用于绘制圆角的图片

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void SLT_show(QImage img);                  //展示到控件上
    void slot_PlayerStateChanged(int state);    //状态切换
    void slot_getTotalTime(qint64 uSec);        //获取视频全部时间
    void slot_TimerTimeOut();                   //获取当前视频时间定时器
    void slot_videoSliderValueChanged(int);     //视频进度通过手动点击改变
    void on_btn_close_clicked();                //关闭窗口
    void on_btn_big_clicked();                  //扩大窗口->回到中心位置
    void on_btn_small_clicked();                //缩小窗口
    void on_btn_home_clicked();                 //首页
    void on_btn_user_clicked();                 //播放页
    void on_btn_play_clicked();                 //用户页
    void on_btn_open_clicked();                 //播放与暂停按钮

    void on_btn_set_clicked();

private:
    Ui::MainWindow *ui;                 //ui界面
    AVPlay* m_play;                     //播放器
    QPoint m_offset;                    //记录指针点击坐标
    bool m_dragging = false;            //标记鼠标是否在标记位置移动窗口
    bool inSpecifiedArea = false;       //标记是否在指定区域内
    SwitchControl* m_swtc;              //连播按钮
    int isStop;                         //暂停状态
    QTimer m_timer;                     //定时器
};
#endif // WIDGET_MAIN_H
