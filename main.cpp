#include "Widget_Main.h"

#include <QApplication>
#include <qDebug>

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    QApplication a(argc, argv);

    Widget_Main w;
    w.show();

    return a.exec();
}
