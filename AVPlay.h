#ifndef AVPLAY_H
#define AVPLAY_H

#include <QThread>
#include <QImage>
#include "PacketQueue.h"

#define MAX_AUDIO_SIZE (1024*16*25*10)        //音频阈值
#define MAX_VIDEO_SIZE (1024*255*25*2)        //视频阈值
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000   //1 second of 48khz 32bit audio
#define SDL_AUDIO_BUFFER_SIZE 1024
#define ERR_STREAM stderr
#define OUT_SAMPLE_RATE 44100
#define FLUSH_DATA "FLUSH"

class AVPlay ;
struct VideoState
{
    AVFormatContext *pFormatCtx;            //相当于视频”文件指针”

    //音频
    AVStream *audio_st;                     //音频流
    PacketQueue *audioq;                    //音频缓冲队列
    AVCodecContext *pAudioCodecCtx ;        //音频解码器信息指针
    int audioStream;                        //视频音频流索引
    double audio_clock;                     //<pts of last decoded frame 音频时钟
    SDL_AudioDeviceID audioID;              //音频 ID
    AVFrame out_frame;                      //设置参数，供音频解码后的 swr_alloc_set_opts 使用。

    // 音频回调函数使用的量
    uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int audio_buf_size = 0;
    unsigned int audio_buf_index = 0;
    AVFrame *audioFrame;

    //视频
    AVStream *video_st;                     //视频流
    PacketQueue *videoq;                    //视频队列
    AVCodecContext *pCodecCtx ;             //音频解码器信息指针
    int videoStream;                        //视频音频流索引
    double video_clock;                     //<pts of last decoded frame 视频时钟
    SDL_Thread *video_tid;                  //视频线程 id
    SDL_TimerID timer_id;                   //无声时定时器id

    // 播放控制的变量
    bool isPause;                           //暂停标志
    bool quit;                              //停止
    bool readFinished;                      //读线程文件读取完毕
    bool readThreadFinished;                //读取线程是否结束
    bool videoThreadFinished;               // 视频线程是否结束

    // 跳转相关的变量
    int seek_req;                           //跳转标志 -- 读线程
    int64_t seek_pos;                       //跳转的位置 -- 微秒
    int seek_flag_audio;                    //跳转标志 -- 用于音频线程中
    int seek_flag_video;                    //跳转标志 -- 用于视频线程中
    double seek_time;                       //跳转的时间(秒) 值和 seek_pos 是一样的
    int64_t start_time;                     //单位 微秒
    VideoState()
    {
        audio_clock = video_clock = start_time = 0;
    }
    AVPlay* m_player;                       //用于调用函数
} ;

class AVPlay : public QThread
{
    Q_OBJECT
public:
    enum PlayerState
    {
        Playing = 0,
        Pause,
        Stop
    };
public:
    explicit AVPlay(QObject *parent = nullptr);
    void run() override;
    void SetFilePath(QString path);
    void packet_queue_flush(PacketQueue *q);
    //播放控制
    void play();
    void pause();
    void stop( bool isWait);

    int64_t getTotalTime();             //获取总时间
    double getCurrentTime();            //获取当前时间
    void seek(int64_t pos);             //跳转
    PlayerState playerState() const;

signals:
    void SIG_GetOneImage(QImage);                    //发给控件帧图片
    void finished();                                 // 信号可以在线程完成后发出
    void SIG_PlayerStateChanged(PlayerState);        //发送更改状态信号
    void SIG_TotalTime(qint64 uSec);                 //发送时间信号
public slots:
    void SendGetOneImage(QImage img);                //发送每一帧图片

private:
    VideoState m_videoState;
    QString m_fileName;
    PlayerState m_playerState;                      //播放状态标志位
};

#endif // AVPLAY_H
