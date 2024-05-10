#include "Widget_Main.h"
#include "ui_widget_main.h"

#include <QEvent>
#include <QFile>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QScreen>
#include <qDebug>

Widget_Main::Widget_Main(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);        // 隐藏标题栏和边框
    setMouseTracking(true);                         // 启用鼠标跟踪
    ui->search->setMouseTracking(true);             //当有多个控件的时候需要所有控件都要设置跟踪，否则不好使
    ui->Title_bar->setMouseTracking(true);
    on_btn_big_clicked();                           //窗口放中间
    qApp->installEventFilter(this);                 //为所有控件添加事件过滤器->为了搜索框
    QFile file(":/style.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    setStyleSheet(in.readAll());                //读取qss并设置样式表

    //投稿 上面图案下面文字
    ui->btn_contribute->setIcon(QIcon(":/image/contribute2.png"));
    ui->btn_contribute->setText(QString::fromLocal8Bit("投稿"));             //如果是乱码就会显示 ...
    ui->btn_contribute->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    //home 左边图案右边文字
    ui->btn_home->setIcon(QIcon(":/image/home.png"));
    ui->btn_home->setText(QString::fromLocal8Bit("首页"));
    ui->btn_home->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->btn_home->setStyleSheet("background-color: rgb(48, 41, 53);color: white;");     //最开始在home窗口
    //my 左边图案右边文字
    ui->btn_user->setIcon(QIcon(":/image/my.png"));
    ui->btn_user->setText(QString::fromLocal8Bit("我的"));
    ui->btn_user->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //paly 左边图案右边文字
    ui->btn_play->setIcon(QIcon(":/image/play.png"));
    ui->btn_play->setText(QString::fromLocal8Bit("播放"));
    ui->btn_play->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //set 左边图案右边文字
    ui->btn_set->setIcon(QIcon(":/image/set.png"));
    ui->btn_set->setText(QString::fromLocal8Bit("设置"));
    ui->btn_set->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //搜索 左边图案后面文字
    ui->btn_search->setIcon(QIcon(":/image/search2.png"));
    ui->btn_search->setText(QString::fromLocal8Bit("搜索"));
    // 设置提示文本
    ui->let_search->setPlaceholderText(QString::fromLocal8Bit("搜索你感兴趣的内容"));
}

Widget_Main::~Widget_Main()
{
    delete ui;
}

QPixmap Widget_Main::roundImage(const QPixmap &src, int radius)
{
    if (src.isNull()) return QPixmap();

    // 如果图片尺寸小于圆角矩形的尺寸，不进行修改
    if (src.width() <= radius * 2 && src.height() <= radius * 2)
        return src;

    // 创建目标图片
    QPixmap rounded(src.size());
    rounded.fill(Qt::transparent);

    // 创建Painter
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);

    // 创建圆角路径，只保留左上角和右上角->有时间可以实现一下
//    QPainterPath path;
//    path.moveTo(src.width(), src.height());
//    path.lineTo(src.width(), 0);
//    path.arcTo(QRectF(0, 0, radius * 2, radius * 2), 90.0, -90.0);
//    path.lineTo(src.width() - radius, src.height());
//    path.arcTo(QRectF(src.width() - radius * 2, 0, radius * 2, radius * 2), 0.0, -90.0);
//    path.lineTo(0, src.height());
//    path.closeSubpath();
    // 创建圆角路径
    QPainterPath path;
    path.addRoundedRect(rounded.rect(), radius, radius);

    // 设置剪裁区域
    painter.setClipPath(path);
    painter.setRenderHints(QPainter::Antialiasing);         //抗锯齿
    painter.setRenderHints(QPainter::SmoothPixmapTransform);//平滑像素图变换
    painter.drawPixmap(rounded.rect(), src);

    return rounded;
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
    //移动窗口
    if (m_dragging)
    {
        // 计算窗口的新位置
        QPoint newpos = event->globalPosition().toPoint() - m_offset;
        move(newpos);
    }

    //搜索框进行变色
    // 获取鼠标的位置
    QPoint mousePos = event->pos();
    // 判断鼠标是否在指定矩形区域内
    if (mousePos.x() >= 600 && mousePos.x() <= 1020 &&
        mousePos.y() >= 20 && mousePos.y() <= 72)
    {
        // 进入指定区域，改变状态并更新界面
        inSpecifiedArea = true;
        update();
    }
    else
    {
        // 离开指定区域，改变状态并更新界面
        inSpecifiedArea = false;
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void Widget_Main::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
    }
}

void Widget_Main::paintEvent(QPaintEvent *event)
{
    // 根据鼠标是否在指定区域内绘制不同的颜色
    if (inSpecifiedArea)
    {
        ui->btn_search->setStyleSheet("background-color: white;color: black;");
        ui->btn_search->setIcon(QIcon(":/image/search1.png")); // 设置按钮图标
    }
    else
    {
        ui->btn_search->setStyleSheet(" background-color: rgba(0, 0, 0, 0);color: white;");
        ui->btn_search->setIcon(QIcon(":/image/search2.png")); // 设置按钮图标
    }

    QWidget::paintEvent(event);
}

bool Widget_Main::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress && watched !=  ui->let_search)
    {
        ui->let_search->setFocus();
        ui->let_search->clearFocus();
        this->setFocus();
    }
    return QObject::eventFilter(watched,event);//千万不能返回true
}

void Widget_Main::SLT_show(QImage img)
{
    QPixmap pixmap = QPixmap::fromImage(img);// 使用 QImage 创建 QPixmap
    pixmap = pixmap.scaled(ui->lb_palyer->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 缩放图片以适应 QLabel 的大小
    // 将原始图片转换为圆角图片
    QPixmap roundedPixmap = roundImage(pixmap, 20); // 20为圆角半径
    ui->lb_palyer->setPixmap(roundedPixmap); // 设置 QLabel 的 pixmap 属性
    ui->lb_palyer->update(); // 更新 QLabel 以显示新的 pixmap
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
    ui->btn_home->setStyleSheet("background-color: rgb(48, 41, 53);color: white;");
    ui->btn_play->setStyleSheet("background-color: transparent;");
    ui->btn_user->setStyleSheet("background-color: transparent;");
}

void Widget_Main::on_btn_play_clicked()
{
    ui->ShwoWindow->setCurrentIndex(1);
    ui->btn_play->setStyleSheet("background-color: rgb(48, 41, 53);color: white;");
    ui->btn_home->setStyleSheet("background-color: transparent;");
    ui->btn_user->setStyleSheet("background-color: transparent;");
}

void Widget_Main::on_btn_user_clicked()
{
    ui->ShwoWindow->setCurrentIndex(2);
    ui->btn_user->setStyleSheet("background-color: rgb(48, 41, 53);color: white;");
    ui->btn_play->setStyleSheet("background-color: transparent;");
    ui->btn_home->setStyleSheet("background-color: transparent;");
}

void Widget_Main::on_btn_open_clicked()
{
    AVPlay* play=new AVPlay;
    connect(play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show); //连接播放器
    play->SetFilePath("E:\\Documents\\01.mp4");
    play->start();
}
