#ifndef FFMPEG_H
#define FFMPEG_H
extern "C"
{
#include <libswresample/swresample.h>                   //获取采样的库
#include <libavdevice/avdevice.h>                       //avdevice:各种设备的输入输出
#include <libavformat/avformat.h>                       //avformat:封装格式处理
#include <libavcodec/avcodec.h>                         //avcodec:编解码(最重要的库)
#include <libswscale/swscale.h>                         //swscale:视频像素数据格式转换
#include <libavutil/imgutils.h>                         //用于计算存储一张图像所需的最小缓冲区大小
#include <libavutil/avutil.h>                           //avutil:工具库（大部分库都需要这个库的支持）
#include <SDL.h>                                        //SDL的使用库
}

#undef main
#endif // FFMPEG_H
