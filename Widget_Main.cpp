#include "Widget_Main.h"
#include "ui_widget_main.h"

#include <QEvent>
#include <QFile>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QScreen>
#include <QSize>
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
    ui->btn_search->setMouseTracking(true);
    ui->let_search->setMouseTracking(true);
    on_btn_big_clicked();                           //窗口放中间
    qApp->installEventFilter(this);                 //为所有控件添加事件过滤器->为了搜索框
    QFile file(":/style.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    setStyleSheet(in.readAll());                    //读取qss并设置样式表

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
    //设置连播按钮颜色
    ui->btn_conopen->setCheckedColor(Qt::red);
    ui->btn_conopen->setBackgroundColor(Qt::white);
    //设置向上和向下按钮的icon
    QPixmap pixmap(":/image/next.png");                          // 创建图标
    QIcon icon;                                                  // 创建 QIcon
    pixmap = pixmap.transformed(QTransform().rotate(90));        //创建旋转图标并旋转90度
    icon.addPixmap(pixmap);
    ui->lb_down->setIcon(icon);
    pixmap = pixmap.transformed(QTransform().rotate(180));       //创建旋转图标并旋转90度
    icon.addPixmap(pixmap);
    ui->lb_on->setIcon(icon);

    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);          //初始化状态
    //连接槽函数
    m_play=new AVPlay;
    connect(m_play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show); //连接播放器
    connect(m_play,&AVPlay::SIG_PlayerStateChanged,this,&Widget_Main::slot_PlayerStateChanged);
    connect(m_play,&AVPlay::SIG_TotalTime,this,&Widget_Main::slot_getTotalTime);
    connect(ui->sl_progree ,&Myslider::SIG_valueChanged,this,&Widget_Main::slot_videoSliderValueChanged);
    connect(&m_timer,&QTimer::timeout,this,&Widget_Main::slot_TimerTimeOut);
    m_timer.setInterval(500);                                                 //超时时间500ms
}

Widget_Main::~Widget_Main()
{
    delete ui;
}

QPixmap Widget_Main::roundImage(const QPixmap &src, int radius)
{
    if (src.isNull()) return QPixmap();

    // 创建目标图片
    QPixmap rounded(src.size());
    rounded.fill(Qt::transparent);

    // 创建Painter
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);

    // 创建圆角路径，只保留左上角和右上角->有时间可以实现一下
    //QPainterPath path;
    //path.moveTo(src.width(), src.height());
    //path.lineTo(src.width(), 0);
    //path.arcTo(QRectF(0, 0, radius * 2, radius * 2), 90.0, -90.0);
    //path.lineTo(src.width() - radius, src.height());
    //path.arcTo(QRectF(src.width() - radius * 2, 0, radius * 2, radius * 2), 0.0, -90.0);
    //path.lineTo(0, src.height());
    //path.closeSubpath();
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
    if (mousePos.x() >= 595 && mousePos.x() <= 1025 &&
        mousePos.y() >= 15 && mousePos.y() <= 77)
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
    // 如果横屏进行修图，否则不改
    QSize pixmapSize = pixmap.size();
    int width = pixmapSize.width();    // 获取宽度
    int height = pixmapSize.height();  // 获取高度
    if (width>height )
    {
        // 将原始图片转换为圆角图片
        QPixmap roundedPixmap = roundImage(pixmap, 20); // 20为圆角半径
        pixmap=roundedPixmap;                           //将修改好的图片进行传递
    }
    pixmap = pixmap.scaled(ui->lb_palyer->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 缩放图片以适应 QLabel 的大小
    ui->lb_palyer->setAlignment(Qt::AlignHCenter|Qt::AlignTop);//水平居中，垂直向上
    ui->lb_palyer->setPixmap(pixmap);                   // 设置 QLabel 的 pixmap 属性
}

void Widget_Main::slot_PlayerStateChanged(int state)
{
    switch( state )
    {
    case AVPlay::PlayerState::Pause:
    {
        ui->btn_open->setIcon(QIcon(":/image/Play _big.png"));
        m_timer.stop();
        this->update();
        isStop = true;
        break;
    }
    case AVPlay::PlayerState::Stop:
    {
        ui->btn_open->setIcon(QIcon(":/image/Play _big.png"));
        m_timer.stop();
        ui->sl_progree->setValue(0);
        ui->lb_totalTime->setText("/00:00");
        ui->lb_currentTime->setText("00:00");
        this->update();
        isStop = true;
        break;
    }
    case AVPlay::PlayerState::Playing:
    {
        ui->btn_open->setIcon(QIcon(":/image/stop.png"));
        m_timer.start();
        this->update();
        isStop = false;
        break;
    }
    }
}

void Widget_Main::slot_getTotalTime(qint64 uSec)
{
    qint64 Sec = uSec/1000000;
    ui->sl_progree->setRange(0,Sec);     //精确到秒
    QString mStr = QString("00%1").arg(Sec/60);
    QString sStr = QString("00%1").arg(Sec%60);
    QString str =QString("/%1:%2").arg(mStr.right(2)).arg(sStr.right(2));
    ui->lb_totalTime->setText(str);
}

void Widget_Main::slot_TimerTimeOut()
{
    if (QObject::sender() == &m_timer)
    {
        qint64 Sec = m_play->getCurrentTime()/1000000;
        ui->sl_progree->setValue(Sec);
        QString mStr = QString("00%1").arg(Sec/60%60);
        QString sStr = QString("00%1").arg(Sec%60);
        QString str =QString("%1:%2").arg(mStr.right(2)).arg(sStr.right(2));
        ui->lb_currentTime->setText(str);
        if(ui->sl_progree->value() == ui->sl_progree->maximum()&& m_play->playerState() == AVPlay::PlayerState::Stop)
        {
            slot_PlayerStateChanged( AVPlay::PlayerState::Stop );
        }
        else if(ui->sl_progree->value() + 1 ==ui->sl_progree->maximum()&& m_play->playerState() == AVPlay::PlayerState::Stop)
        {
            slot_PlayerStateChanged( AVPlay::PlayerState::Stop );
        }
    }
}

void Widget_Main::slot_videoSliderValueChanged(int value)
{
    if( QObject::sender() == ui->sl_progree)
    {
        m_play->seek((qint64)value*1000000); //value 秒
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
    //如果是暂停，或者停止(没有文件加载)
    if(m_play->playerState()==AVPlay::PlayerState::Pause||m_play->playerState()==AVPlay::PlayerState::Stop)
    {
        m_play->play();
        if(m_play->playerState()==AVPlay::PlayerState::Stop)
        {
            //m_play->SetFilePath("E:\\Documents\\02.mp4");    //用于测试 后期连播等，需要重写
            m_play->SetFilePath("rtmp://192.168.194.131:1935/vod/7.mp4");//点播
        }
        //切换状态
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
    }
    else if(m_play->playerState()==AVPlay::PlayerState::Playing)
    {
       m_play->pause();
       //切换状态
       slot_PlayerStateChanged(AVPlay::PlayerState::Pause);
    }
}

void Widget_Main::on_btn_set_clicked()
{
    QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("设置界面仍处于更新状态，后续可能会进行完成，也可能不了了之了"));

}

