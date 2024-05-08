#include "Widget_Main.h"
#include "ui_widget_main.h"

#include <QPixmap>

Widget_Main::Widget_Main(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget_Main)
{
    ui->setupUi(this);
    m_play=new AVPlay;
    connect(m_play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show);
    m_play->start();
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

