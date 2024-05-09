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
    connect(m_play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show); //连接播放器

    setWindowFlags(Qt::FramelessWindowHint);        // 隐藏标题栏和边框
    on_btn_big_clicked();                           //窗口放中间


    //投稿 上面图案下面文字
    ui->btn_contribute->setIcon(QIcon(":/image/contribute1.png"));
    ui->btn_contribute->setText(QString::fromLocal8Bit("投稿"));             //如果是乱码就会显示 ...
    ui->btn_contribute->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    //搜索 左边图案后面文字
    ui->btn_search->setIcon(QIcon(":/image/search1.png"));
    ui->btn_search->setText(QString::fromLocal8Bit("搜索"));             //如果是乱码就会显示 ...

    //开启线程
    //m_play->SetFilePath(" ");
    //m_play->start();
}

Widget_Main::~Widget_Main()
{
    delete ui;
}

void Widget_Main::SLT_show(QImage img)
{
    QPixmap pixmap = QPixmap::fromImage(img);// 使用 QImage 创建 QPixmap
    //pixmap = pixmap.scaled(ui->lb_vid->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 缩放图片以适应 QLabel 的大小

    //ui->lb_vid->setPixmap(pixmap); // 设置 QLabel 的 pixmap 属性
    //ui->lb_vid->update(); // 更新 QLabel 以显示新的 pixmap
}

void Widget_Main::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() < 85)// 只在标题栏区域拖动
    {
        m_dragging = true;
        // 记录鼠标按下时的位置与窗口左上角的偏移
        m_offset = event->pos();
    }
}

void Widget_Main::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging)
    {
        // 计算窗口的新位置
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
    // 获取主屏幕对象
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QRect screenGeometry = primaryScreen->geometry();

    // 获取屏幕的宽度和高度
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 获取窗口的宽度和高度
    int windowWidth = width();
    int windowHeight = height();

    // 计算窗口在屏幕中心的位置
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    // 将窗口移动到屏幕中心
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




