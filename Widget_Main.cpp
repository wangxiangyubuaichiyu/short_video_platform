#include "Widget_Main.h"
#include "ui_widget_main.h"

#include <QMouseEvent>
#include <QPixmap>
#include <QScreen>

Widget_Main::Widget_Main(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_play=new AVPlay;
    connect(m_play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show); //���Ӳ�����

    setWindowFlags(Qt::FramelessWindowHint);        // ���ر������ͱ߿�
    on_btn_big_clicked();                           //���ڷ��м�


    //Ͷ�� ����ͼ����������
    ui->btn_contribute->setIcon(QIcon(":/image/contribute1.png"));
    ui->btn_contribute->setText(QString::fromLocal8Bit("Ͷ��"));             //���������ͻ���ʾ ...
    ui->btn_contribute->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    //���� ���ͼ����������
    ui->btn_search->setIcon(QIcon(":/image/search1.png"));
    ui->btn_search->setText(QString::fromLocal8Bit("����"));             //���������ͻ���ʾ ...

    //�����߳�
    //m_play->SetFilePath(" ");
    //m_play->start();
}

Widget_Main::~Widget_Main()
{
    delete ui;
}

void Widget_Main::SLT_show(QImage img)
{
    QPixmap pixmap = QPixmap::fromImage(img);// ʹ�� QImage ���� QPixmap
    //pixmap = pixmap.scaled(ui->lb_vid->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // ����ͼƬ����Ӧ QLabel �Ĵ�С

    //ui->lb_vid->setPixmap(pixmap); // ���� QLabel �� pixmap ����
    //ui->lb_vid->update(); // ���� QLabel ����ʾ�µ� pixmap
}

void Widget_Main::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() < 85)// ֻ�ڱ����������϶�
    {
        m_dragging = true;
        // ��¼��갴��ʱ��λ���봰�����Ͻǵ�ƫ��
        m_offset = event->pos();
    }
}

void Widget_Main::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging)
    {
        // ���㴰�ڵ���λ��
        QPoint newpos = event->globalPosition().toPoint() - m_offset;
        move(newpos);
    }
}

void Widget_Main::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
    }
}


void Widget_Main::on_btn_close_clicked()
{
    this->close();
}


void Widget_Main::on_btn_big_clicked()
{
    // ��ȡ����Ļ����
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QRect screenGeometry = primaryScreen->geometry();

    // ��ȡ��Ļ�Ŀ�Ⱥ͸߶�
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // ��ȡ���ڵĿ�Ⱥ͸߶�
    int windowWidth = width();
    int windowHeight = height();

    // ���㴰������Ļ���ĵ�λ��
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    // �������ƶ�����Ļ����
    setGeometry(x, y, windowWidth, windowHeight);
}


void Widget_Main::on_btn_small_clicked()
{
    this->showMinimized();
}


void Widget_Main::on_btn_home_clicked()
{
    ui->ShwoWindow->setCurrentIndex(0);
}

void Widget_Main::on_btn_play_clicked()
{
    ui->ShwoWindow->setCurrentIndex(1);
}

void Widget_Main::on_btn_user_clicked()
{
    ui->ShwoWindow->setCurrentIndex(2);
}




