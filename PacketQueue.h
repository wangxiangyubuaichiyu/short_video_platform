#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H
#include "ffmpeg.h"

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
#endif // PACKETQUEUE_H
