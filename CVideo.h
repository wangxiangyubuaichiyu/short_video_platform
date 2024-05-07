#ifndef CVIDEO_H
#define CVIDEO_H

#include <QObject>
#include <QImage>
#include "ffmpeg.h"

class CVideo : public QObject
{
    Q_OBJECT
signals:
    void SIG_GetOneImage(QImage); // 发送图像的信号

public:
    explicit CVideo(QObject *parent = nullptr);
    ~CVideo();

    bool open(const QString& url); // 打开视频
    void start(); // 开始播放

private slots:
    void decodeNextFrame(); // 解码下一帧

private:
    AVFormatContext* m_pFormatCtx; // 格式上下文
    int m_videoStreamIndex; // 视频流索引
    AVCodecContext* m_pCodecCtx; // 解码器上下文
    const AVCodec* m_pCodec; // 解码器
    AVFrame* m_pFrame; // 视频帧
    AVFrame* m_pFrameRGB; // 转换后的RGB格式视频帧
    uint8_t* m_buffer; // 图像缓冲区
    struct SwsContext* m_pSwsCtx; // 视频像素格式转换上下文
    double m_timeBase;
    int64_t m_lastPts;
};

#endif // CVIDEO_H
