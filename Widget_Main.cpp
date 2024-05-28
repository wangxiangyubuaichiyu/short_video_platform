#include "ui_widget_main.h"
#include "Widget_Main.h"

#include <QPainterPath>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileInfo>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QScreen>
#include <qDebug>
#include <QEvent>
#include <QSize>
#include <QFile>


Widget_Main::Widget_Main(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    //初始化变量
    m_play=new AVPlay;
    m_list=new ViList;
    m_js=new Json;

    //设置界面样式
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
    ui->btn_contribute->setText(QString::fromLocal8Bit("投稿"));                      //如果是乱码就会显示 ...
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
    //设置大的暂停按钮
    QPixmap pixmapn(":/image/Play _big.png");
    pixmapn = pixmapn.scaled(QSize(64, 64), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lb_pause->setPixmap(pixmapn);
    ui->lb_pause->resize(pixmapn.size());                        // 调整 QLabel 大小以适应图片

    //连接槽函数
    connect(m_js,&Json::dataEncoded,this,&Widget_Main::sendDate);               //封装好数据要发送给服务端
    connect(m_js,&Json::dataDecoded,this,&Widget_Main::addList);                //解析好数据要放到链表中
    connect(m_play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show);    //连接播放器
    connect(m_play,&AVPlay::SIG_PlayerStateChanged,this,&Widget_Main::slot_PlayerStateChanged);
    connect(m_play,&AVPlay::SIG_TotalTime,this,&Widget_Main::slot_getTotalTime);
    connect(m_play,&AVPlay::finished,this,&Widget_Main::playMode);
    connect(&m_timer,&QTimer::timeout,this,&Widget_Main::slot_TimerTimeOut);
    connect(ui->sl_progree ,&Myslider::SIG_valueChanged,this,&Widget_Main::slot_videoSliderValueChanged);
    connect(ui->lb_palyer,&Mylabel::clicked,this,[&](){on_btn_open_clicked();});
    connect(ui->lw_recommendlist,&MyListWidget::needopen,this,&Widget_Main::home2Open);
    connect(ui->btn_conopen,&SwitchControl::toggled,this,[&](bool is){m_playmode=is;});
    //先加载控件再运行主程序的，所以刚加载的时候MyListWidget::header是空的，所以放到主窗口进行初始化。
    connect(ui->lw_recommendlist,&MyListWidget::getList,this,[&](ViList*& list){//获取到链表
        list=m_list;
    });

    m_timer.setInterval(500);                                                    //超时时间500ms
    //加载视频需要4的倍数
    for(int i=1;i<=20;i++)//测试性放20个url
    {
        m_js->decodeData(QString("{\"url\":\"rtmp://192.168.194.131:1935/vod/%1.mp4\"}").arg(i));
    }

    //设置初始化状态
    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);          // 初始化播放状态
    ui->lw_recommendlist->loadMoreItems();                       // 设置好主页开始时视频
    m_NowNode=m_list->check(0);                                  // 给第一个视频地址
    ui->lb_pause->hide();
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

    // 创建路径 只有左上角和右上角修改为圆角
    QPainterPath path;
    path.moveTo(radius, 0);                                                        //从左上角向右移动到圆角的起点。
    path.lineTo(rounded.width() - radius, 0);                                      //从起点画一条水平线到右上角的圆角起点。
    path.arcTo(rounded.width() - 2 * radius, 0, 2 * radius, 2 * radius, 90, -90);  //画一个圆弧到右上角。
    path.lineTo(rounded.width(), rounded.height());                                //从右上角的圆弧终点画一条垂直线到右下角。
    path.lineTo(0, rounded.height());                                              //从右下角画一条水平线到左下角。
    path.lineTo(0, radius);                                                        //从左下角画一条垂直线到左上角的圆角起点。
    path.arcTo(0, 0, 2 * radius, 2 * radius, 180, -90);                            //画一个圆弧到左上角。
    path.closeSubpath();                                                           //闭合路径。

    // 创建圆角路径
    //QPainterPath path;
    //path.addRoundedRect(rounded.rect(), radius, radius);
    // 设置剪裁区域
    painter.setClipPath(path);
    painter.setRenderHints(QPainter::Antialiasing);         //抗锯齿
    painter.setRenderHints(QPainter::SmoothPixmapTransform);//平滑像素图变换
    painter.drawPixmap(rounded.rect(), src);

    return rounded;
}

QImage Widget_Main::getFirstImage(QString filePath)
{
    AVFormatContext *pFormatCtx = nullptr;
    int videoStreamIndex = -1;
    AVCodecContext *pCodecCtx = nullptr;
    AVFrame *pFrame = nullptr;
    AVFrame *pFrameRGB = nullptr;
    AVPacket packet;
    struct SwsContext *sws_ctx = nullptr;
    QImage image;

    av_register_all();

    if (avformat_open_input(&pFormatCtx, filePath.toStdString().c_str(), nullptr, nullptr) != 0)
    {
        qDebug() << "Couldn't open file.";
        return image;
    }

    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        qDebug() << "Couldn't find stream information.";
        avformat_close_input(&pFormatCtx);
        return image;
    }

    videoStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVCodecParameters *pCodecPar = pFormatCtx->streams[videoStreamIndex]->codecpar;
    AVCodec *pCodec = avcodec_find_decoder(pCodecPar->codec_id);
    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pCodecPar);
    avcodec_open2(pCodecCtx, pCodec, nullptr);
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);

    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                             pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,
                             SWS_BILINEAR, nullptr, nullptr, nullptr);

    //取第一帧可能有问题，所以需要取到第一张图片才能结束。
    while (av_read_frame(pFormatCtx, &packet) >= 0)
    {
        if (packet.stream_index == videoStreamIndex)
        {
            if (avcodec_send_packet(pCodecCtx, &packet) == 0)
            {
                if (avcodec_receive_frame(pCodecCtx, pFrame) == 0)
                {
                    sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize,
                              0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                    image = QImage(pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB888).copy();
                    break; // 只需要第一帧
                }
            }
        }
        av_packet_unref(&packet);
    }

    av_free(buffer);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtx);
    sws_freeContext(sws_ctx);
    return image;
}

QString Widget_Main::getFilename(const QString &filePath)
{
    //加载库
    av_register_all();

    //获取名字
    AVFormatContext *formatContext = avformat_alloc_context();
    avformat_open_input(&formatContext, filePath.toStdString().c_str(), nullptr, nullptr);
    avformat_find_stream_info(formatContext, nullptr);
    QString url = QString::fromUtf8(formatContext->url);
    QFileInfo fileInfo(url);
    QString filename = fileInfo.fileName();

    //卸载库
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    return filename;
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

void Widget_Main::addList(const QMap<QString, QVariant> &data)
{
    QString url=data["url"].toString();
    VNode* p=new VNode;
    p->url=url;

    //获取名字，并且进行字符截取
    p->name=getFilename(url);
    int lastDotIndex = p->name.lastIndexOf('.');               // 查找最后一个点号的位置
    if (lastDotIndex != -1)                                    // 如果找到了点号
    {
        p->name = p->name.left(lastDotIndex);                  // 截断到点号之前
    }

    p->FirstImage=QPixmap::fromImage(getFirstImage(url));
    p->FirstImage = roundImage(p->FirstImage, 20);             // 20为圆角半径
    p->Id=num++;
    p->next=NULL;
    m_list->push_back(p);
}

void Widget_Main::sendDate(const QString &json)
{
    //qDebug() << json;
}

void Widget_Main::home2Open(VNode*& t)
{
    //将界面清空
    SLT_show(t->FirstImage.toImage());
    m_NowNode=t;

    if(m_play->playerState()==AVPlay::PlayerState::Stop)
    {
        m_play->SetFilePath(t->url);
        //切换状态
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
    }
    else
    {
        m_play->stop(true);
        m_play->SetFilePath(t->url);
        //切换状态
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
    }
    // 跳转到播放界面
    on_btn_play_clicked();
}

void Widget_Main::playMode()
{
    //播放完成后关闭界面
    m_play->stop(true);
    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);

    if(m_playmode)
    {
        if(m_NowNode->next==NULL)
        {
            // 添加节点
            emit getListNode();
        }
        m_NowNode=m_NowNode->next;
    }
    m_play->SetFilePath(m_NowNode->url);
    //切换状态
    slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
}

void Widget_Main::slot_PlayerStateChanged(int state)
{
    switch( state )
    {
        case AVPlay::PlayerState::Pause:
        {
            ui->btn_open->setIcon(QIcon(":/image/Play _big.png"));
            m_timer.stop();           
            break;
        }
        case AVPlay::PlayerState::Stop:
        {
            ui->btn_open->setIcon(QIcon(":/image/Play _big.png"));
            m_timer.stop();
            ui->sl_progree->setValue(0);
            ui->lb_totalTime->setText("/00:00");
            ui->lb_currentTime->setText("00:00");
            break;
        }
        case AVPlay::PlayerState::Playing:
        {
            ui->btn_open->setIcon(QIcon(":/image/stop.png"));
            m_timer.start();
            break;
        }
    }
    this->update();
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
    if(m_play->playerState()==AVPlay::PlayerState::Playing)
    {
        m_play->pause();
        slot_PlayerStateChanged(AVPlay::PlayerState::Pause);
    }
}

void Widget_Main::on_btn_play_clicked()
{
    ui->ShwoWindow->setCurrentIndex(1);
    ui->btn_play->setStyleSheet("background-color: rgb(48, 41, 53);color: white;");
    ui->btn_home->setStyleSheet("background-color: transparent;");
    ui->btn_user->setStyleSheet("background-color: transparent;");
    if(m_play->playerState()==AVPlay::PlayerState::Pause)
    {
        m_play->play();
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
    }
    else if(m_play->playerState()==AVPlay::PlayerState::Stop)
    {
        m_play->SetFilePath(m_NowNode->url);
        //切换状态
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
    }
}

void Widget_Main::on_btn_user_clicked()
{
    ui->ShwoWindow->setCurrentIndex(2);
    ui->btn_user->setStyleSheet("background-color: rgb(48, 41, 53);color: white;");
    ui->btn_play->setStyleSheet("background-color: transparent;");
    ui->btn_home->setStyleSheet("background-color: transparent;");
    if(m_play->playerState()==AVPlay::PlayerState::Playing)
    {
        m_play->pause();
        slot_PlayerStateChanged(AVPlay::PlayerState::Pause);
    }
}

void Widget_Main::on_btn_open_clicked()
{
    //如果是暂停
    if(m_play->playerState()==AVPlay::PlayerState::Pause)
    {
        m_play->play();
        //切换状态
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
        ui->lb_pause->hide();
    }
    else if(m_play->playerState()==AVPlay::PlayerState::Playing)
    {
       m_play->pause();
       //切换状态
       slot_PlayerStateChanged(AVPlay::PlayerState::Pause);
       ui->lb_pause->show();
    }
}

void Widget_Main::on_btn_set_clicked()
{
    QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("设置界面仍处于更新状态，后续可能会进行完成，也可能不了了之了"));
}

void Widget_Main::on_lb_on_clicked()
{
    if(m_NowNode->Id==0)return;

    m_play->stop(true);
    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);
    ui->lb_pause->hide();

    m_NowNode=m_list->check(m_NowNode->Id-1);
    m_play->SetFilePath(m_NowNode->url);
    //切换状态
    slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
}

void Widget_Main::on_lb_down_clicked()
{
    m_play->stop(true);
    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);
    ui->lb_pause->hide();

    if(m_NowNode->next==NULL)
    {
       // 添加节点
       emit getListNode();
    }
    m_NowNode=m_NowNode->next;
    m_play->SetFilePath(m_NowNode->url);
    //切换状态
    slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
}
