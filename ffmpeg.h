#ifndef FFMPEG_H
#define FFMPEG_H
extern "C"
{
#include <libswresample/swresample.h>                   //��ȡ�����Ŀ�
#include <libavdevice/avdevice.h>                       //avdevice:�����豸���������
#include <libavformat/avformat.h>                       //avformat:��װ��ʽ����
#include <libavcodec/avcodec.h>                         //avcodec:�����(����Ҫ�Ŀ�)
#include <libswscale/swscale.h>                         //swscale:��Ƶ�������ݸ�ʽת��
#include <libavutil/imgutils.h>                         //���ڼ���洢һ��ͼ���������С��������С
#include <libavutil/avutil.h>                           //avutil:���߿⣨�󲿷ֿⶼ��Ҫ������֧�֣�
#include <SDL.h>                                        //SDL��ʹ�ÿ�
}

#undef main
#endif // FFMPEG_H
