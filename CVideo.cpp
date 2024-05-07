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
    //�汾����4,ffmpeg���Զ�ע�ᣬ�����ٵ���
    av_register_all(); // ע�����еĽ������͸�ʽ
}

CVideo::~CVideo()
{
    if (m_pFrame)
        av_frame_free(&m_pFrame); // �ͷ���Ƶ֡
    if (m_pFrameRGB)
        av_frame_free(&m_pFrameRGB); // �ͷ�RGB��ʽ����Ƶ֡
    if (m_buffer)
        av_free(m_buffer); // �ͷ�ͼ�񻺳���
    if (m_pCodecCtx)
        avcodec_free_context(&m_pCodecCtx); // �ͷŽ�����������
    if (m_pFormatCtx)
        avformat_close_input(&m_pFormatCtx); // �ر���Ƶ�ļ�
}

bool CVideo::open(const QString& url)
{
    m_pFormatCtx = avformat_alloc_context(); // �����ʽ������
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

    m_pFrame = av_frame_alloc(); // ������Ƶ֡
    m_pFrameRGB = av_frame_alloc(); // ����RGB��ʽ����Ƶ֡
    m_buffer = static_cast<uint8_t*>(av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_pCodecCtx->width, m_pCodecCtx->height, 1)));

    av_image_fill_arrays(m_pFrameRGB->data, m_pFrameRGB->linesize, m_buffer, AV_PIX_FMT_RGB24, m_pCodecCtx->width, m_pCodecCtx->height, 1);

    m_pSwsCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
                               m_pCodecCtx->width, m_pCodecCtx->height, AV_PIX_FMT_RGB24,
                               SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!m_pSwsCtx)
    {
        return false;
    }

    // ��ȡ��Ƶ����ʱ���׼
    AVRational timeBase = m_pFormatCtx->streams[m_videoStreamIndex]->time_base;
    m_timeBase = static_cast<double>(timeBase.num) / timeBase.den; // ��ʱ���׼ת��Ϊ��
    return true;
}

void CVideo::start()
{
    // ��ʼ����Ͳ�����Ƶ
    decodeNextFrame();
}

void CVideo::decodeNextFrame()
{
    AVPacket packet;
    int response = av_read_frame(m_pFormatCtx, &packet);
    if (response < 0) {
        // ��Ƶ���Ž���
        return;
    }

    if (packet.stream_index != m_videoStreamIndex) {
        av_packet_unref(&packet);
        decodeNextFrame(); // ����������һ֡
        return;
    }

    response = avcodec_send_packet(m_pCodecCtx, &packet);
    if (response < 0) {
        av_packet_unref(&packet);
        decodeNextFrame(); // ����������һ֡
        return;
    }

    response = avcodec_receive_frame(m_pCodecCtx, m_pFrame);
    if (response < 0) {
        av_packet_unref(&packet);
        decodeNextFrame(); // ����������һ֡
        return;
    }

    sws_scale(m_pSwsCtx, m_pFrame->data, m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrameRGB->data, m_pFrameRGB->linesize);
    QImage image(m_pFrameRGB->data[0], m_pCodecCtx->width, m_pCodecCtx->height, QImage::Format_RGB888);
    emit SIG_GetOneImage(image);

    av_packet_unref(&packet);

    // ���㵱ǰ֡����һ֡��ʱ���
    int64_t currentPts = m_pFrame->pts;
    if (m_lastPts == 0) {
        m_lastPts = currentPts;
    }
    int64_t frameDuration = (currentPts - m_lastPts) * m_timeBase * 1000; // ��ʱ���ת��Ϊ����
    m_lastPts = currentPts;

    // �ȴ�ֱ����һ֡����ʾʱ��
    std::this_thread::sleep_for(std::chrono::milliseconds(frameDuration));

    // ����������һ֡
    decodeNextFrame();
}

