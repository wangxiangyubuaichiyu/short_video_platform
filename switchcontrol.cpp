#include "switchcontrol.h"

#include <QPainterPath>
#include <QMouseEvent>
#include <QPainter>

SwitchControl::SwitchControl(QWidget *parent)
    : QWidget(parent),
    m_nHeight(16),
    m_bChecked(false),
    m_radius(8.0),
    m_nMargin(3),
    m_checkedColor(0, 150, 136),
    m_thumbColor(Qt::white),
    m_disabledColor(190, 190, 190),
    m_background(Qt::black)
{
    // ��껬�������״ - ����
    setCursor(Qt::PointingHandCursor);

    // �����źŲ�
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

// ���ƿ���
void SwitchControl::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    QColor background;
    QColor thumbColor;
    qreal dOpacity;
    if (isEnabled())                          // ����״̬
    {
        if (m_bChecked)                       // ��״̬
        {
            background = m_checkedColor;
            thumbColor=Qt::white;             //�ð�ťԲ���ǰ�ɫ
            //thumbColor = m_checkedColor;
            dOpacity = 0.600;
        }
        else                                  //�ر�״̬
        {
            background = m_background;
            thumbColor = m_thumbColor;
            dOpacity = 0.800;
        }
    }
    else                                      // ������״̬
    {
        background = m_background;
        dOpacity = 0.260;
        thumbColor = m_disabledColor;
    }
    // ���ƴ���Բ
    painter.setBrush(background);
    painter.setOpacity(dOpacity);
    path.addRoundedRect(QRectF(m_nMargin, m_nMargin, width() - 2 * m_nMargin, height() - 2 * m_nMargin), m_radius, m_radius);
    painter.drawPath(path.simplified());

    // ����С��Բ
    painter.setBrush(thumbColor);
    painter.setOpacity(1.0);
    //��Բ����
    //painter.drawEllipse(QRectF(m_nX - (m_nHeight / 2), m_nY - (m_nHeight / 2), height(), height()));
    //��Բ�ڿ���                                       +4��΢������ɾȥ
    painter.drawEllipse(QRectF(m_nX - (m_nHeight / 2)+4, m_nY - (m_nHeight / 6)-1 , height()/2, height()/2));
}

// ��갴���¼�
void SwitchControl::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled())
    {
        if (event->buttons() & Qt::LeftButton)
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
}

// ����ͷ��¼� - �л�����״̬������toggled()�ź�
void SwitchControl::mouseReleaseEvent(QMouseEvent *event)
{
    if (isEnabled())
    {
        if ((event->type() == QMouseEvent::MouseButtonRelease) && (event->button() == Qt::LeftButton))
        {
            event->accept();
            m_bChecked = !m_bChecked;
            emit toggled(m_bChecked);
            m_timer.start(10);
        }
        else
        {
            event->ignore();
        }
    }
}

// ��С�ı��¼�
void SwitchControl::resizeEvent(QResizeEvent *event)
{
    m_nX = m_nHeight / 2;
    m_nY = m_nHeight / 2;
    QWidget::resizeEvent(event);
}

// Ĭ�ϴ�С
QSize SwitchControl::sizeHint() const
{
    return minimumSizeHint();
}

// ��С��С
QSize SwitchControl::minimumSizeHint() const
{
    return QSize(2 * (m_nHeight + m_nMargin), m_nHeight + 2 * m_nMargin);
}

// �л�״̬ - ����
void SwitchControl::onTimeout()
{
    if (m_bChecked)
    {
        //��Բ����
        //m_nX += 1;
        //if (m_nX >= width() - m_nHeight)
        //{
        //    m_timer.stop();
        //}
        //��Բ�ڿ���->����ʹ��qt�����������ã������õ��Ƕ�ʱ��������
        m_nX += 1;
        //                              -2��΢��������΢����һ��ɾȥ
        if (m_nX >= width()- m_nHeight/2-2)
        {
            m_timer.stop();
        }
    }
    else
    {
        //��Բ����
        //m_nX -= 1;
        //if (m_nX <= m_nHeight / 2)
        //{
        //    m_timer.stop();
        //}
        //��Բ�ڿ���
        m_nX -= 1;
        //                                    -4��΢��������΢����һ��ɾȥ
        if (m_nX <= m_nHeight / 2+ m_nHeight/4-4)
        {
            m_timer.stop();
        }
    }
    update();
}

// ���ؿ���״̬ - �򿪣�true �رգ�false
bool SwitchControl::isToggled() const
{
    return m_bChecked;
}

// ���ÿ���״̬
void SwitchControl::setToggle(bool checked)
{
    m_bChecked = checked;
    m_timer.start(10);
}

// ���ñ�����ɫ
void SwitchControl::setBackgroundColor(QColor color)
{
    m_background = color;
}

// ����ѡ����ɫ
void SwitchControl::setCheckedColor(QColor color)
{
    m_checkedColor = color;
}

// ���ò�������ɫ
void SwitchControl::setDisbaledColor(QColor color)
{
    m_disabledColor = color;
}
