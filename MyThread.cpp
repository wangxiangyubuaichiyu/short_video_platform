#include "MyThread.h"
#include <qDebug>
MyThread::MyThread(QObject *parent)
    : QThread{parent}
{
    vd=new CVideo;
    ad=new CAudio;
}
void MyThread::run()
{
    // 音视频操作
    connect(vd, &CVideo::SIG_GetOneImage, this, &MyThread::SIG_sand);
    vd->open("E:\\Documents\\01.mp4");
    vd->start();
    qDebug()<<"~~~~~~~~";
    ad->playAudio("E:\\Documents\\01.mp4");
    // 线程完成后发出信号
    //emit finished();
}
