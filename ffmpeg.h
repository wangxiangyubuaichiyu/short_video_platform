#ifndef FFMPEG_H
#define FFMPEG_H
extern "C"
{
//avcodec:�����(����Ҫ�Ŀ�)
#include <libavcodec/avcodec.h>
//avformat:��װ��ʽ����
#include <libavformat/avformat.h>
//swscale:��Ƶ�������ݸ�ʽת��
#include <libswscale/swscale.h>
//avdevice:�����豸���������
#include <libavdevice/avdevice.h>
//avutil:���߿⣨�󲿷ֿⶼ��Ҫ������֧�֣�
#include <libavutil/avutil.h>
//���ڼ���洢һ��ͼ���������С��������С
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
//SDL��ʹ�ÿ�
#include <SDL.h>
}
#undef main
#endif // FFMPEG_H
