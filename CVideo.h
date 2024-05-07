#ifndef CVIDEO_H
#define CVIDEO_H

#include <QObject>
#include <QImage>
#include "ffmpeg.h"

class CVideo : public QObject
{
    Q_OBJECT
signals:
    void SIG_GetOneImage(QImage); // ����ͼ����ź�

public:
    explicit CVideo(QObject *parent = nullptr);
    ~CVideo();

    bool open(const QString& url); // ����Ƶ
    void start(); // ��ʼ����

private slots:
    void decodeNextFrame(); // ������һ֡

private:
    AVFormatContext* m_pFormatCtx; // ��ʽ������
    int m_videoStreamIndex; // ��Ƶ������
    AVCodecContext* m_pCodecCtx; // ������������
    const AVCodec* m_pCodec; // ������
    AVFrame* m_pFrame; // ��Ƶ֡
    AVFrame* m_pFrameRGB; // ת�����RGB��ʽ��Ƶ֡
    uint8_t* m_buffer; // ͼ�񻺳���
    struct SwsContext* m_pSwsCtx; // ��Ƶ���ظ�ʽת��������
    double m_timeBase;
    int64_t m_lastPts;
};

#endif // CVIDEO_H
