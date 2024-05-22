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
    ui->btn_contribute->setText(QString::fromLocal8Bit("Ͷ��"));             //���������ͻ���ʾ ...
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

    slot_PlayerStateChanged(AVPlay::PlayerState::Stop);          //��ʼ��״̬
    //���Ӳۺ���
    m_play=new AVPlay;
    connect(m_play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show); //���Ӳ�����
    connect(m_play,&AVPlay::SIG_PlayerStateChanged,this,&Widget_Main::slot_PlayerStateChanged);
    connect(m_play,&AVPlay::SIG_TotalTime,this,&Widget_Main::slot_getTotalTime);
    connect(ui->sl_progree ,&Myslider::SIG_valueChanged,this,&Widget_Main::slot_videoSliderValueChanged);
    connect(&m_timer,&QTimer::timeout,this,&Widget_Main::slot_TimerTimeOut);
    m_timer.setInterval(500);                                                 //��ʱʱ��500ms
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

    // ����Բ��·����ֻ�������ϽǺ����Ͻ�->��ʱ�����ʵ��һ��
    //QPainterPath path;
    //path.moveTo(src.width(), src.height());
    //path.lineTo(src.width(), 0);
    //path.arcTo(QRectF(0, 0, radius * 2, radius * 2), 90.0, -90.0);
    //path.lineTo(src.width() - radius, src.height());
    //path.arcTo(QRectF(src.width() - radius * 2, 0, radius * 2, radius * 2), 0.0, -90.0);
    //path.lineTo(0, src.height());
    //path.closeSubpath();
    // ����Բ��·��
    QPainterPath path;
    path.addRoundedRect(rounded.rect(), radius, radius);

    // ���ü�������
    painter.setClipPath(path);
    painter.setRenderHints(QPainter::Antialiasing);         //�����
    painter.setRenderHints(QPainter::SmoothPixmapTransform);//ƽ������ͼ�任
    painter.drawPixmap(rounded.rect(), src);

    return rounded;
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
    //�������ͣ������ֹͣ(û���ļ�����)
    if(m_play->playerState()==AVPlay::PlayerState::Pause||m_play->playerState()==AVPlay::PlayerState::Stop)
    {
        m_play->play();
        if(m_play->playerState()==AVPlay::PlayerState::Stop)
        {
            //m_play->SetFilePath("E:\\Documents\\02.mp4");    //���ڲ��� ���������ȣ���Ҫ��д
            m_play->SetFilePath("rtmp://192.168.194.131:1935/vod/7.mp4");//�㲥
        }
        //�л�״̬
        slot_PlayerStateChanged(AVPlay::PlayerState::Playing);
    }
    else if(m_play->playerState()==AVPlay::PlayerState::Playing)
    {
       m_play->pause();
       //�л�״̬
       slot_PlayerStateChanged(AVPlay::PlayerState::Pause);
    }
}

void Widget_Main::on_btn_set_clicked()
{
    QMessageBox::about(NULL, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("���ý����Դ��ڸ���״̬���������ܻ������ɣ�Ҳ���ܲ�����֮��"));

}

