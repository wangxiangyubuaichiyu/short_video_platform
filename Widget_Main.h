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
    //拖动窗口可以进行拖拽
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private slots:
    void SLT_show(QImage img);   //展示到控件上
    void on_btn_close_clicked(); //关闭窗口
    void on_btn_big_clicked();   //扩大窗口->回到中心位置
    void on_btn_small_clicked(); //缩小窗口

    void on_btn_home_clicked(); //首页
    void on_btn_user_clicked(); //播放页
    void on_btn_play_clicked(); //用户页
    void on_btn_open_clicked(); //播放与暂停按钮

private:
    Ui::MainWindow *ui;  //ui界面
    AVPlay* m_play;      //播放器
    QPoint m_offset;     //记录指针点击坐标
    bool m_dragging = false;

    //HomeWindow homeWnd;

};
#endif // WIDGET_MAIN_H
