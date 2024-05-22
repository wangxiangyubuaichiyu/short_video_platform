#ifndef AVPLAY_H
#define AVPLAY_H

#include <QThread>
#include <QImage>
#include "PacketQueue.h"

#define MAX_AUDIO_SIZE (1024*16*25*10)        //��Ƶ��ֵ
#define MAX_VIDEO_SIZE (1024*255*25*2)        //��Ƶ��ֵ
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000   //1 second of 48khz 32bit audio
#define SDL_AUDIO_BUFFER_SIZE 1024
#define ERR_STREAM stderr
#define OUT_SAMPLE_RATE 44100
#define FLUSH_DATA "FLUSH"

class AVPlay ;
struct VideoState
{
    AVFormatContext *pFormatCtx;            //�൱����Ƶ���ļ�ָ�롱

    //��Ƶ
    AVStream *audio_st;                     //��Ƶ��
    PacketQueue *audioq;                    //��Ƶ�������
    AVCodecContext *pAudioCodecCtx ;        //��Ƶ��������Ϣָ��
    int audioStream;                        //��Ƶ��Ƶ������
    double audio_clock;                     //<pts of last decoded frame ��Ƶʱ��
    SDL_AudioDeviceID audioID;              //��Ƶ ID
    AVFrame out_frame;                      //���ò���������Ƶ������ swr_alloc_set_opts ʹ�á�

    // ��Ƶ�ص�����ʹ�õ���
    uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int audio_buf_size = 0;
    unsigned int audio_buf_index = 0;
    AVFrame *audioFrame;

    //��Ƶ
    AVStream *video_st;                     //��Ƶ��
    PacketQueue *videoq;                    //��Ƶ����
    AVCodecContext *pCodecCtx ;             //��Ƶ��������Ϣָ��
    int videoStream;                        //��Ƶ��Ƶ������
    double video_clock;                     //<pts of last decoded frame ��Ƶʱ��
    SDL_Thread *video_tid;                  //��Ƶ�߳� id
    SDL_TimerID timer_id;                   //����ʱ��ʱ��id

    // ���ſ��Ƶı���
    bool isPause;                           //��ͣ��־
    bool quit;                              //ֹͣ
    bool readFinished;                      //���߳��ļ���ȡ���
    bool readThreadFinished;                //��ȡ�߳��Ƿ����
    bool videoThreadFinished;               // ��Ƶ�߳��Ƿ����

    // ��ת��صı���
    int seek_req;                           //��ת��־ -- ���߳�
    int64_t seek_pos;                       //��ת��λ�� -- ΢��
    int seek_flag_audio;                    //��ת��־ -- ������Ƶ�߳���
    int seek_flag_video;                    //��ת��־ -- ������Ƶ�߳���
    double seek_time;                       //��ת��ʱ��(��) ֵ�� seek_pos ��һ����
    int64_t start_time;                     //��λ ΢��
    VideoState()
    {
        audio_clock = video_clock = start_time = 0;
    }
    AVPlay* m_player;                       //���ڵ��ú���
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
    //���ſ���
    void play();
    void pause();
    void stop( bool isWait);

    int64_t getTotalTime();             //��ȡ��ʱ��
    double getCurrentTime();            //��ȡ��ǰʱ��
    void seek(int64_t pos);             //��ת
    PlayerState playerState() const;

signals:
    void SIG_GetOneImage(QImage);                    //�����ؼ�֡ͼƬ
    void finished();                                 // �źſ������߳���ɺ󷢳�
    void SIG_PlayerStateChanged(PlayerState);        //���͸���״̬�ź�
    void SIG_TotalTime(qint64 uSec);                 //����ʱ���ź�
public slots:
    void SendGetOneImage(QImage img);                //����ÿһ֡ͼƬ

private:
    VideoState m_videoState;
    QString m_fileName;
    PlayerState m_playerState;                      //����״̬��־λ
};

#endif // AVPLAY_H
