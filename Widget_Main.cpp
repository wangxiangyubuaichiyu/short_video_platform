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
    QPixmap pixmap = QPixmap::fromImage(img);// ʹ�� QImage ���� QPixmap
    pixmap = pixmap.scaled(ui->lb_vid->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // ����ͼƬ����Ӧ QLabel �Ĵ�С

    ui->lb_vid->setPixmap(pixmap); // ���� QLabel �� pixmap ����
    ui->lb_vid->update(); // ���� QLabel ����ʾ�µ� pixmap
}

