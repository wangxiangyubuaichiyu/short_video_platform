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
    QPixmap roundImage(const QPixmap &src, int radius); // 函数用于绘制圆角的图片
    QImage getFirstImage(QString filePath);             //获取第一帧画面
    QString getFilename(const QString &filePath);       //获取视频名字

protected:

    //重写事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void getListNode();                                 //发送信号给tcp，缺少节点了

private slots:
    void playMode();                                    //设置是否连播
    void SLT_show(QImage img);                          //展示到控件上
    void addList(const QMap<QString, QVariant>& data);  //获取数据
    void sendDate(const QString& json);                 //发送数据
    void home2Open(VNode*& t);                          //播放传来的视频
    void slot_PlayerStateChanged(int state);            //状态切换
    void slot_getTotalTime(qint64 uSec);                //获取视频全部时间
    void slot_TimerTimeOut();                           //获取当前视频时间定时器
    void slot_videoSliderValueChanged(int);             //视频进度通过手动点击改变
    void on_btn_close_clicked();                        //关闭窗口
    void on_btn_big_clicked();                          //扩大窗口->回到中心位置
    void on_btn_small_clicked();                        //缩小窗口
    void on_btn_home_clicked();                         //首页
    void on_btn_user_clicked();                         //播放页
    void on_btn_play_clicked();                         //用户页
    void on_btn_open_clicked();                         //播放与暂停按钮
    void on_btn_set_clicked();                          //设置界面
    void on_lb_on_clicked();                            //上一个视频
    void on_lb_down_clicked();                          //下一个视频

private:
    Ui::MainWindow *ui;                                 //ui界面
    AVPlay* m_play;                                     //播放器
    QPoint m_offset;                                    //记录指针点击坐标
    bool m_dragging = false;                            //标记鼠标是否在标记位置移动窗口
    bool inSpecifiedArea = false;                       //标记是否在指定区域内
    SwitchControl* m_swtc;                              //连播按钮
    QTimer m_timer;                                     //定时器
    ViList* m_list;                                     //视频存放列表
    VNode* m_NowNode;                                   //当前节点
    Json* m_js;                                         //json解析器
    int num=0;                                          //视频数量
    bool m_playmode=false;                              //播放标志
};
#endif // WIDGET_MAIN_H
