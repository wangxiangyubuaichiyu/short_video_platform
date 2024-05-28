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
    //��ʼ������
    m_play=new AVPlay;
    m_list=new ViList;
    m_js=new Json;

    //���ý�����ʽ
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);        // ���ر������ͱ߿�
    setMouseTracking(true);                         // ����������
    ui->search->setMouseTracking(true);             //���ж���ؼ���ʱ����Ҫ���пؼ���Ҫ���ø��٣����򲻺�ʹ
    ui->Title_bar->setMouseTracking(true);
    ui->btn_search->setMouseTracking(true);
    ui->let_search->setMouseTracking(true);
    on_btn_big_clicked();                           //���ڷ��м�
    qApp->installEventFilter(this);                 //Ϊ���пؼ�����¼�������->Ϊ��������
    QFile file(":/style.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    setStyleSheet(in.readAll());                    //��ȡqss��������ʽ��

    //Ͷ�� ����ͼ����������
    ui->btn_contribute->setIcon(QIcon(":/image/contribute2.png"));
    ui->btn_contribute->setText(QString::fromLocal8Bit("Ͷ��"));                      //���������ͻ���ʾ ...
    ui->btn_contribute->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    //home ���ͼ���ұ�����
    ui->btn_home->setIcon(QIcon(":/image/home.png"));
    ui->btn_home->setText(QString::fromLocal8Bit("��ҳ"));
    ui->btn_home->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->btn_home->setStyleSheet("background-color: rgb(48, 41, 53);color: white;");     //�ʼ��home����
    //my ���ͼ���ұ�����
    ui->btn_user->setIcon(QIcon(":/image/my.png"));
    ui->btn_user->setText(QString::fromLocal8Bit("�ҵ�"));
    ui->btn_user->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //paly ���ͼ���ұ�����
    ui->btn_play->setIcon(QIcon(":/image/play.png"));
    ui->btn_play->setText(QString::fromLocal8Bit("����"));
    ui->btn_play->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //set ���ͼ���ұ�����
    ui->btn_set->setIcon(QIcon(":/image/set.png"));
    ui->btn_set->setText(QString::fromLocal8Bit("����"));
    ui->btn_set->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //���� ���ͼ����������
    ui->btn_search->setIcon(QIcon(":/image/search2.png"));
    ui->btn_search->setText(QString::fromLocal8Bit("����"));
    // ������ʾ�ı�
    ui->let_search->setPlaceholderText(QString::fromLocal8Bit("���������Ȥ������"));
    //����������ť��ɫ
    ui->btn_conopen->setCheckedColor(Qt::red);
    ui->btn_conopen->setBackgroundColor(Qt::white);
    //�������Ϻ����°�ť��icon
    QPixmap pixmap(":/image/next.png");                          // ����ͼ��
    QIcon icon;                                                  // ���� QIcon
    pixmap = pixmap.transformed(QTransform().rotate(90));        //������תͼ�겢��ת90��
    icon.addPixmap(pixmap);
    ui->lb_down->setIcon(icon);
    pixmap = pixmap.transformed(QTransform().rotate(180));       //������תͼ�겢��ת90��
    icon.addPixmap(pixmap);
    ui->lb_on->setIcon(icon);
    //���ô����ͣ��ť
    QPixmap pixmapn(":/image/Play _big.png");
    pixmapn = pixmapn.scaled(QSize(64, 64), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lb_pause->setPixmap(pixmapn);
    ui->lb_pause->resize(pixmapn.size());                        // ���� QLabel ��С����ӦͼƬ

    //���Ӳۺ���
    connect(m_js,&Json::dataEncoded,this,&Widget_Main::sendDate);               //��װ������Ҫ���͸������
    connect(m_js,&Json::dataDecoded,this,&Widget_Main::addList);                //����������Ҫ�ŵ�������
    connect(m_play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show);    //���Ӳ�����
    connect(m_play,&AVPlay::SIG_PlayerStateChanged,this,&Widget_Main::slot_PlayerStateChanged);
    connect(m_play,&AVPlay::SIG_TotalTime,this,&Widget_Main::slot_getTotalTime);
    connect(m_play,&AVPlay::finished,this,&Widget_Main::playMode);
    connect(&m_timer,&QTimer::timeout,this,&Widget_Main::slot_TimerTimeOut);
    connect(ui->sl_progree ,&Myslider::SIG_valueChanged,this,&Widget_Main::slot_videoSliderValueChanged);
    connect(ui->lb_palyer,&Mylabel::clicked,this,[&](){on_btn_open_clicked();});
    connect(ui->lw_recommendlist,&MyListWidget::needopen,this,&Widget_Main::home2Open);
    connect(ui->btn_conopen,&SwitchControl::toggled,this,[&](bool is){m_playmode=is;});
    //�ȼ��ؿؼ�������������ģ����Ըռ��ص�ʱ��MyListWidget::header�ǿյģ����Էŵ������ڽ��г�ʼ����
    connect(ui->lw_recommendlist,&MyListWidget::getList,this,[&](ViList*& list){//��ȡ������
        list=m_list;
    });

    m_timer.setInterval(500);                                                    //��ʱʱ��500ms
    //������Ƶ��Ҫ4�ı���
    for(int i=1;i<=20;i++)//�����Է�20��url
    {
        m_js->decodeData(QString("{\"url\":\"rtmp://192.168.194.131:1935/vod/%1.mp4\"}").arg(i));
    }

    //���ó�ʼ��״̬
    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);          // ��ʼ������״̬
    ui->lw_recommendlist->loadMoreItems();                       // ���ú���ҳ��ʼʱ��Ƶ
    m_NowNode=m_list->check(0);                                  // ����һ����Ƶ��ַ
    ui->lb_pause->hide();
}

Widget_Main::~Widget_Main()
{
    delete ui;
}

QPixmap Widget_Main::roundImage(const QPixmap &src, int radius)
{
    if (src.isNull()) return QPixmap();

    // ����Ŀ��ͼƬ
    QPixmap rounded(src.size());
    rounded.fill(Qt::transparent);

    // ����Painter
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);

    // ����·�� ֻ�����ϽǺ����Ͻ��޸�ΪԲ��
    QPainterPath path;
    path.moveTo(radius, 0);                                                        //�����Ͻ������ƶ���Բ�ǵ���㡣
    path.lineTo(rounded.width() - radius, 0);                                      //����㻭һ��ˮƽ�ߵ����Ͻǵ�Բ����㡣
    path.arcTo(rounded.width() - 2 * radius, 0, 2 * radius, 2 * radius, 90, -90);  //��һ��Բ�������Ͻǡ�
    path.lineTo(rounded.width(), rounded.height());                                //�����Ͻǵ�Բ���յ㻭һ����ֱ�ߵ����½ǡ�
    path.lineTo(0, rounded.height());                                              //�����½ǻ�һ��ˮƽ�ߵ����½ǡ�
    path.lineTo(0, radius);                                                        //�����½ǻ�һ����ֱ�ߵ����Ͻǵ�Բ����㡣
    path.arcTo(0, 0, 2 * radius, 2 * radius, 180, -90);                            //��һ��Բ�������Ͻǡ�
    path.closeSubpath();                                                           //�պ�·����

    // ����Բ��·��
    //QPainterPath path;
    //path.addRoundedRect(rounded.rect(), radius, radius);
    // ���ü�������
    painter.setClipPath(path);
    painter.setRenderHints(QPainter::Antialiasing);         //�����
    painter.setRenderHints(QPainter::SmoothPixmapTransform);//ƽ������ͼ�任
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

    //ȡ��һ֡���������⣬������Ҫȡ����һ��ͼƬ���ܽ�����
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
                    break; // ֻ��Ҫ��һ֡
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
    //���ؿ�
    av_register_all();

    //��ȡ����
    AVFormatContext *formatContext = avformat_alloc_context();
    avformat_open_input(&formatContext, filePath.toStdString().c_str(), nullptr, nullptr);
    avformat_find_stream_info(formatContext, nullptr);
    QString url = QString::fromUtf8(formatContext->url);
    QFileInfo fileInfo(url);
    QString filename = fileInfo.fileName();

    //ж�ؿ�
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    return filename;
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
    //�ƶ�����
    if (m_dragging)
    {
        // ���㴰�ڵ���λ��
        QPoint newpos = event->globalPosition().toPoint() - m_offset;
        move(newpos);
    }

    //��������б�ɫ
    // ��ȡ����λ��
    QPoint mousePos = event->pos();
    // �ж�����Ƿ���ָ������������
    if (mousePos.x() >= 595 && mousePos.x() <= 1025 &&
        mousePos.y() >= 15 && mousePos.y() <= 77)
    {
        // ����ָ�����򣬸ı�״̬�����½���
        inSpecifiedArea = true;
        update();
    }
    else
    {
        // �뿪ָ�����򣬸ı�״̬�����½���
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
    // ��������Ƿ���ָ�������ڻ��Ʋ�ͬ����ɫ
    if (inSpecifiedArea)
    {
        ui->btn_search->setStyleSheet("background-color: white;color: black;");
        ui->btn_search->setIcon(QIcon(":/image/search1.png")); // ���ð�ťͼ��
    }
    else
    {
        ui->btn_search->setStyleSheet(" background-color: rgba(0, 0, 0, 0);color: white;");
        ui->btn_search->setIcon(QIcon(":/image/search2.png")); // ���ð�ťͼ��
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
    return QObject::eventFilter(watched,event);//ǧ���ܷ���true
}

void Widget_Main::SLT_show(QImage img)
{
    QPixmap pixmap = QPixmap::fromImage(img);// ʹ�� QImage ���� QPixmap
    // �������������ͼ�����򲻸�
    QSize pixmapSize = pixmap.size();
    int width = pixmapSize.width();    // ��ȡ���
    int height = pixmapSize.height();  // ��ȡ�߶�
    if (width>height )
    {
        // ��ԭʼͼƬת��ΪԲ��ͼƬ
        QPixmap roundedPixmap = roundImage(pixmap, 20); // 20ΪԲ�ǰ뾶
        pixmap=roundedPixmap;                           //���޸ĺõ�ͼƬ���д���
    }
    pixmap = pixmap.scaled(ui->lb_palyer->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // ����ͼƬ����Ӧ QLabel �Ĵ�С
    ui->lb_palyer->setAlignment(Qt::AlignHCenter|Qt::AlignTop);//ˮƽ���У���ֱ����
    ui->lb_palyer->setPixmap(pixmap);                   // ���� QLabel �� pixmap ����
}

void Widget_Main::addList(const QMap<QString, QVariant> &data)
{
    QString url=data["url"].toString();
    VNode* p=new VNode;
    p->url=url;

    //��ȡ���֣����ҽ����ַ���ȡ
    p->name=getFilename(url);
    int lastDotIndex = p->name.lastIndexOf('.');               // �������һ����ŵ�λ��
    if (lastDotIndex != -1)                                    // ����ҵ��˵��
    {
        p->name = p->name.left(lastDotIndex);                  // �ضϵ����֮ǰ
    }

    p->FirstImage=QPixmap::fromImage(getFirstImage(url));
    p->FirstImage = roundImage(p->FirstImage, 20);             // 20ΪԲ�ǰ뾶
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
    //���������
    SLT_show(t->FirstImage.toImage());
    m_NowNode=t;

    if(m_play->playerState()==AVPlay::PlayerState::Stop)
    {
        m_play->SetFilePath(t->url);
        //�л�״̬
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
    }
    else
    {
        m_play->stop(true);
        m_play->SetFilePath(t->url);
        //�л�״̬
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
    }
    // ��ת�����Ž���
    on_btn_play_clicked();
}

void Widget_Main::playMode()
{
    //������ɺ�رս���
    m_play->stop(true);
    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);

    if(m_playmode)
    {
        if(m_NowNode->next==NULL)
        {
            // ��ӽڵ�
            emit getListNode();
        }
        m_NowNode=m_NowNode->next;
    }
    m_play->SetFilePath(m_NowNode->url);
    //�л�״̬
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
    ui->sl_progree->setRange(0,Sec);     //��ȷ����
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
        m_play->seek((qint64)value*1000000); //value ��
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
        //�л�״̬
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
    //�������ͣ
    if(m_play->playerState()==AVPlay::PlayerState::Pause)
    {
        m_play->play();
        //�л�״̬
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
        ui->lb_pause->hide();
    }
    else if(m_play->playerState()==AVPlay::PlayerState::Playing)
    {
       m_play->pause();
       //�л�״̬
       slot_PlayerStateChanged(AVPlay::PlayerState::Pause);
       ui->lb_pause->show();
    }
}

void Widget_Main::on_btn_set_clicked()
{
    QMessageBox::about(NULL, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("���ý����Դ��ڸ���״̬���������ܻ������ɣ�Ҳ���ܲ�����֮��"));
}

void Widget_Main::on_lb_on_clicked()
{
    if(m_NowNode->Id==0)return;

    m_play->stop(true);
    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);
    ui->lb_pause->hide();

    m_NowNode=m_list->check(m_NowNode->Id-1);
    m_play->SetFilePath(m_NowNode->url);
    //�л�״̬
    slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
}

void Widget_Main::on_lb_down_clicked()
{
    m_play->stop(true);
    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);
    ui->lb_pause->hide();

    if(m_NowNode->next==NULL)
    {
       // ��ӽڵ�
       emit getListNode();
    }
    m_NowNode=m_NowNode->next;
    m_play->SetFilePath(m_NowNode->url);
    //�л�״̬
    slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
}
