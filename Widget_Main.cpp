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

    setWindowFlags(Qt::FramelessWindowHint);        // ���ر������ͱ߿�
    setMouseTracking(true);                         // ����������
    ui->search->setMouseTracking(true);             //���ж���ؼ���ʱ����Ҫ���пؼ���Ҫ���ø��٣����򲻺�ʹ
    ui->Title_bar->setMouseTracking(true);
    on_btn_big_clicked();                           //���ڷ��м�
    qApp->installEventFilter(this);                 //Ϊ���пؼ�����¼�������->Ϊ��������
    QFile file(":/style.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    setStyleSheet(in.readAll());                //��ȡqss��������ʽ��

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
}

Widget_Main::~Widget_Main()
{
    delete ui;
}

QPixmap Widget_Main::roundImage(const QPixmap &src, int radius)
{
    if (src.isNull()) return QPixmap();

    // ���ͼƬ�ߴ�С��Բ�Ǿ��εĳߴ磬�������޸�
    if (src.width() <= radius * 2 && src.height() <= radius * 2)
        return src;

    // ����Ŀ��ͼƬ
    QPixmap rounded(src.size());
    rounded.fill(Qt::transparent);

    // ����Painter
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);

    // ����Բ��·����ֻ�������ϽǺ����Ͻ�->��ʱ�����ʵ��һ��
//    QPainterPath path;
//    path.moveTo(src.width(), src.height());
//    path.lineTo(src.width(), 0);
//    path.arcTo(QRectF(0, 0, radius * 2, radius * 2), 90.0, -90.0);
//    path.lineTo(src.width() - radius, src.height());
//    path.arcTo(QRectF(src.width() - radius * 2, 0, radius * 2, radius * 2), 0.0, -90.0);
//    path.lineTo(0, src.height());
//    path.closeSubpath();
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
    if (mousePos.x() >= 600 && mousePos.x() <= 1020 &&
        mousePos.y() >= 20 && mousePos.y() <= 72)
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
    pixmap = pixmap.scaled(ui->lb_palyer->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // ����ͼƬ����Ӧ QLabel �Ĵ�С
    // ��ԭʼͼƬת��ΪԲ��ͼƬ
    QPixmap roundedPixmap = roundImage(pixmap, 20); // 20ΪԲ�ǰ뾶
    ui->lb_palyer->setPixmap(roundedPixmap); // ���� QLabel �� pixmap ����
    ui->lb_palyer->update(); // ���� QLabel ����ʾ�µ� pixmap
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
    AVPlay* play=new AVPlay;
    connect(play, &AVPlay::SIG_GetOneImage, this, &Widget_Main::SLT_show); //���Ӳ�����
    play->SetFilePath("E:\\Documents\\01.mp4");
    play->start();
}
