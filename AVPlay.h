#ifndef AVPLAY_H
#define AVPLAY_H

#include <QThread>
#include <QImage>
#include "PacketQueue.h"

#define MAX_AUDIO_SIZE (1024*16*25*10)        //音频阈值
#define MAX_VIDEO_SIZE (1024*255*25*2)        //视频阈值
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000   //1 second of 48khz 32bit audio
#define SDL_AUDIO_BUFFER_SIZE 1024
#define FILE_NAME " "
#define ERR_STREAM stderr
#define OUT_SAMPLE_RATE 44100

class AVPlay ;
struct VideoState
{
    AVFormatContext *pFormatCtx;                       //相当于视频”文件指针”

    //音频
    AVStream *audio_st;                                //音频流
    PacketQueue *audioq;                               //音频缓冲队列
    AVCodecContext *pAudioCodecCtx ;                   //音频解码器信息指针
    int audioStream;                                   //视频音频流索引
    double audio_clock;                                //<pts of last decoded frame 音频时钟
    SDL_AudioDeviceID audioID;                         //音频 ID
    AVFrame out_frame;                                 //设置参数，供音频解码后的 swr_alloc_set_opts 使用。

    //音频回调函数使用的量
    uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int audio_buf_size = 0;
    unsigned int audio_buf_index = 0;
    AVFrame *audioFrame;

    //视频
    AVStream *video_st;                                //视频流
    PacketQueue *videoq;                               //视频队列
    AVCodecContext *pCodecCtx ;                        //音频解码器信息指针
    int videoStream;                                   //视频音频流索引
    double video_clock;                                //<pts of last decoded frame 视频时钟
    SDL_Thread *video_tid;                             //视频线程 id

    //播放控制
    int quit;
    int64_t start_time;                                //单位 微秒

    VideoState()
    {
        audio_clock = video_clock = start_time = 0;
    }

    AVPlay* m_player;                                  //用于调用函数
} ;
class AVPlay : public QThread
{
    Q_OBJECT
public:
    explicit AVPlay(QObject *parent = nullptr);
    void run() override;

signals:
    void SIG_GetOneImage(QImage);                     //发给控件帧图片
    void finished();                                  // 信号可以在线程完成后发出

public slots:
    void SendGetOneImage(QImage img);

private:
    VideoState m_videoState;
    QString m_fileName;
};

#endif // AVPLAY_H
