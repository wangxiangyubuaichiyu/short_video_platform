#ifndef FFMPEG_H
#define FFMPEG_H
extern "C"
{
//avcodec:编解码(最重要的库)
#include <libavcodec/avcodec.h>
//avformat:封装格式处理
#include <libavformat/avformat.h>
//swscale:视频像素数据格式转换
#include <libswscale/swscale.h>
//avdevice:各种设备的输入输出
#include <libavdevice/avdevice.h>
//avutil:工具库（大部分库都需要这个库的支持）
#include <libavutil/avutil.h>
//用于计算存储一张图像所需的最小缓冲区大小
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
//SDL的使用库
#include <SDL.h>
}
#undef main
#endif // FFMPEG_H
