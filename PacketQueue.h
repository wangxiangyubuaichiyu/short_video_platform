#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H
#include "ffmpeg.h"

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
#endif // PACKETQUEUE_H
