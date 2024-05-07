#include "CVideo.h"

#include <QThread>
#include <QTime>
#include <thread>
CVideo::CVideo(QObject *parent) : QObject(parent),
    m_pFormatCtx(nullptr),
    m_videoStreamIndex(-1),
    m_pCodecCtx(nullptr),
    m_pCodec(nullptr),
    m_pFrame(nullptr),
    m_pFrameRGB(nullptr),
    m_buffer(nullptr),
    m_pSwsCtx(nullptr)
{
    //版本大于4,ffmpeg会自动注册，不用再调用
    av_register_all(); // 注册所有的解码器和格式
}

CVideo::~CVideo()
{
    if (m_pFrame)
        av_frame_free(&m_pFrame); // 释放视频帧
    if (m_pFrameRGB)
        av_frame_free(&m_pFrameRGB); // 释放RGB格式的视频帧
    if (m_buffer)
        av_free(m_buffer); // 释放图像缓冲区
    if (m_pCodecCtx)
        avcodec_free_context(&m_pCodecCtx); // 释放解码器上下文
    if (m_pFormatCtx)
        avformat_close_input(&m_pFormatCtx); // 关闭视频文件
}

bool CVideo::open(const QString& url)
{
    m_pFormatCtx = avformat_alloc_context(); // 分配格式上下文
    if (!m_pFormatCtx)
    {
        return false;
    }

    if (avformat_open_input(&m_pFormatCtx, url.toStdString().c_str(), nullptr, nullptr) != 0)
    {
        return false;
    }

    if (avformat_find_stream_info(m_pFormatCtx, nullptr) < 0)
    {
        return false;
    }

    for (unsigned i = 0; i < m_pFormatCtx->nb_streams; ++i)
    {
        if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_videoStreamIndex = i;
            break;
        }
    }

    if (m_videoStreamIndex == -1)
    {
        return false;
    }
    m_pCodec = avcodec_find_decoder(m_pFormatCtx->streams[m_videoStreamIndex]->codecpar->codec_id);
    if (!m_pCodec)
    {
        return false;
    }

    m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
    if (!m_pCodecCtx)
    {
        return false;
    }

    if (avcodec_parameters_to_context(m_pCodecCtx, m_pFormatCtx->streams[m_videoStreamIndex]->codecpar) < 0)
    {
        return false;
    }

    if (avcodec_open2(m_pCodecCtx, m_pCodec, nullptr) < 0)
    {
        return false;
    }

    m_pFrame = av_frame_alloc(); // 分配视频帧
    m_pFrameRGB = av_frame_alloc(); // 分配RGB格式的视频帧
    m_buffer = static_cast<uint8_t*>(av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_pCodecCtx->width, m_pCodecCtx->height, 1)));

    av_image_fill_arrays(m_pFrameRGB->data, m_pFrameRGB->linesize, m_buffer, AV_PIX_FMT_RGB24, m_pCodecCtx->width, m_pCodecCtx->height, 1);

    m_pSwsCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
                               m_pCodecCtx->width, m_pCodecCtx->height, AV_PIX_FMT_RGB24,
                               SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!m_pSwsCtx)
    {
        return false;
    }

    // 获取视频流的时间基准
    AVRational timeBase = m_pFormatCtx->streams[m_videoStreamIndex]->time_base;
    m_timeBase = static_cast<double>(timeBase.num) / timeBase.den; // 将时间基准转换为秒
    return true;
}

void CVideo::start()
{
    // 开始解码和播放视频
    decodeNextFrame();
}

void CVideo::decodeNextFrame()
{
    AVPacket packet;
    int response = av_read_frame(m_pFormatCtx, &packet);
    if (response < 0) {
        // 视频播放结束
        return;
    }

    if (packet.stream_index != m_videoStreamIndex) {
        av_packet_unref(&packet);
        decodeNextFrame(); // 继续解码下一帧
        return;
    }

    response = avcodec_send_packet(m_pCodecCtx, &packet);
    if (response < 0) {
        av_packet_unref(&packet);
        decodeNextFrame(); // 继续解码下一帧
        return;
    }

    response = avcodec_receive_frame(m_pCodecCtx, m_pFrame);
    if (response < 0) {
        av_packet_unref(&packet);
        decodeNextFrame(); // 继续解码下一帧
        return;
    }

    sws_scale(m_pSwsCtx, m_pFrame->data, m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrameRGB->data, m_pFrameRGB->linesize);
    QImage image(m_pFrameRGB->data[0], m_pCodecCtx->width, m_pCodecCtx->height, QImage::Format_RGB888);
    emit SIG_GetOneImage(image);

    av_packet_unref(&packet);

    // 计算当前帧和下一帧的时间差
    int64_t currentPts = m_pFrame->pts;
    if (m_lastPts == 0) {
        m_lastPts = currentPts;
    }
    int64_t frameDuration = (currentPts - m_lastPts) * m_timeBase * 1000; // 将时间差转换为毫秒
    m_lastPts = currentPts;

    // 等待直到下一帧的显示时间
    std::this_thread::sleep_for(std::chrono::milliseconds(frameDuration));

    // 继续解码下一帧
    decodeNextFrame();
}

