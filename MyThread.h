#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include "CVideo.h"
#include "CAudio.h"
class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = nullptr);
    void run() override;

signals:
    void SIG_sand(QImage);
    void finished(); // �źſ������߳���ɺ󷢳�
private:
    CVideo* vd;
    CAudio* ad;
};

#endif // MYTHREAD_H
