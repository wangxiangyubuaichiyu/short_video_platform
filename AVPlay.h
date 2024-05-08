#ifndef AVPLAY_H
#define AVPLAY_H

#include <QThread>
#include <QImage>
#include "PacketQueue.h"

#define MAX_AUDIO_SIZE (1024*16*25*10)        //��Ƶ��ֵ
#define MAX_VIDEO_SIZE (1024*255*25*2)        //��Ƶ��ֵ
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000   //1 second of 48khz 32bit audio
#define SDL_AUDIO_BUFFER_SIZE 1024
#define FILE_NAME " "
#define ERR_STREAM stderr
#define OUT_SAMPLE_RATE 44100

class AVPlay ;
struct VideoState
{
    AVFormatContext *pFormatCtx;                       //�൱����Ƶ���ļ�ָ�롱

    //��Ƶ
    AVStream *audio_st;                                //��Ƶ��
    PacketQueue *audioq;                               //��Ƶ�������
    AVCodecContext *pAudioCodecCtx ;                   //��Ƶ��������Ϣָ��
    int audioStream;                                   //��Ƶ��Ƶ������
    double audio_clock;                                //<pts of last decoded frame ��Ƶʱ��
    SDL_AudioDeviceID audioID;                         //��Ƶ ID
    AVFrame out_frame;                                 //���ò���������Ƶ������ swr_alloc_set_opts ʹ�á�

    //��Ƶ�ص�����ʹ�õ���
    uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int audio_buf_size = 0;
    unsigned int audio_buf_index = 0;
    AVFrame *audioFrame;

    //��Ƶ
    AVStream *video_st;                                //��Ƶ��
    PacketQueue *videoq;                               //��Ƶ����
    AVCodecContext *pCodecCtx ;                        //��Ƶ��������Ϣָ��
    int videoStream;                                   //��Ƶ��Ƶ������
    double video_clock;                                //<pts of last decoded frame ��Ƶʱ��
    SDL_Thread *video_tid;                             //��Ƶ�߳� id

    //���ſ���
    int quit;
    int64_t start_time;                                //��λ ΢��

    VideoState()
    {
        audio_clock = video_clock = start_time = 0;
    }

    AVPlay* m_player;                                  //���ڵ��ú���
} ;
class AVPlay : public QThread
{
    Q_OBJECT
public:
    explicit AVPlay(QObject *parent = nullptr);
    void run() override;

signals:
    void SIG_GetOneImage(QImage);                     //�����ؼ�֡ͼƬ
    void finished();                                  // �źſ������߳���ɺ󷢳�

public slots:
    void SendGetOneImage(QImage img);

private:
    VideoState m_videoState;
    QString m_fileName;
};

#endif // AVPLAY_H
