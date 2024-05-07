#include "Widget_Main.h"
#include "ui_widget_main.h"
#include "CVideo.h"

#include <QPixmap>
Widget_Main::Widget_Main(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget_Main)
{
    ui->setupUi(this);
    th=new MyThread;
    connect(th, &MyThread::SIG_sand, this, &Widget_Main::SLT_show);
    th->start();
}

Widget_Main::~Widget_Main()
{
    delete ui;
}

void Widget_Main::SLT_show(QImage img)
{
    QPixmap pixmap = QPixmap::fromImage(img);// 使用 QImage 创建 QPixmap
    pixmap = pixmap.scaled(ui->lb_vid->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 缩放图片以适应 QLabel 的大小

    ui->lb_vid->setPixmap(pixmap); // 设置 QLabel 的 pixmap 属性
    ui->lb_vid->update(); // 更新 QLabel 以显示新的 pixmap
}

