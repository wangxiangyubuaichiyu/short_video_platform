#ifndef CAUDIO_H
#define CAUDIO_H

#include <QObject>
#include <queue>
#include "ffmpeg.h"
#include <SDL2/SDL.h> // ����SDLͷ�ļ�
struct PacketQueue
{
    AVPacketList *first_pkt; //��ͷ��һ�� packet, ע�����Ͳ��� AVPacket
    AVPacketList *last_pkt; //��β packet
    int nb_packets; // paket ����
    int size; //
    SDL_mutex *mutex; //
    SDL_cond *cond; // ��������

};
void packet_queue_init(PacketQueue *queue);
int packet_queue_put(PacketQueue *queue, AVPacket *packet);
int packet_queue_get(PacketQueue *queue, AVPacket *pakcet, int block);

class CAudio : public QObject
{
    Q_OBJECT
public:
    CAudio(QObject *parent = nullptr);
    ~CAudio();
    //�ص�����
    static void audio_callback(void *userdata, Uint8 *stream, int len);
    //���뺯��
    static int audio_decode_frame(AVCodecContext *pcodec_ctx, uint8_t *audio_buf, int buf_size);
    //�� auto_stream
    int find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int* audio_stream);

    void playAudio(const char* filename);

private:
    AVFormatContext *pFormatCtx;
    int audioStream = -1;//��������Ҫ����������
    AVCodecContext *pCodecCtx = NULL;//������
    AVCodec *pCodec = NULL; //������
    AVPacket packet ; // ����ǰ������
    AVFrame *pframe = NULL; //����֮�������
    char* filename;
    //SDL
    SDL_AudioSpec wanted_spec; //SDL ��Ƶ����
    SDL_AudioSpec spec ; //SDL ��Ƶ����
};

#endif // CAUDIO_H
