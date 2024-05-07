#ifndef CAUDIO_H
#define CAUDIO_H

#include <QObject>
#include <queue>
#include "ffmpeg.h"
#include <SDL2/SDL.h> // 包含SDL头文件
struct PacketQueue
{
    AVPacketList *first_pkt; //队头的一个 packet, 注意类型不是 AVPacket
    AVPacketList *last_pkt; //队尾 packet
    int nb_packets; // paket 个数
    int size; //
    SDL_mutex *mutex; //
    SDL_cond *cond; // 条件变量

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
    //回调函数
    static void audio_callback(void *userdata, Uint8 *stream, int len);
    //解码函数
    static int audio_decode_frame(AVCodecContext *pcodec_ctx, uint8_t *audio_buf, int buf_size);
    //找 auto_stream
    int find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int* audio_stream);

    void playAudio(const char* filename);

private:
    AVFormatContext *pFormatCtx;
    int audioStream = -1;//解码器需要的流的索引
    AVCodecContext *pCodecCtx = NULL;//解码器
    AVCodec *pCodec = NULL; //解码器
    AVPacket packet ; // 解码前的数据
    AVFrame *pframe = NULL; //解码之后的数据
    char* filename;
    //SDL
    SDL_AudioSpec wanted_spec; //SDL 音频设置
    SDL_AudioSpec spec ; //SDL 音频设置
};

#endif // CAUDIO_H
