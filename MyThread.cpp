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
    // ����Ƶ����
    connect(vd, &CVideo::SIG_GetOneImage, this, &MyThread::SIG_sand);
    vd->open("E:\\Documents\\01.mp4");
    vd->start();
    qDebug()<<"~~~~~~~~";
    ad->playAudio("E:\\Documents\\01.mp4");
    // �߳���ɺ󷢳��ź�
    //emit finished();
}
