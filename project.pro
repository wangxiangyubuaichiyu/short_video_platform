QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH +=$$PWD/ffmpeglib/include \
              $$PWD/SDL/include

LIBS += $$PWD/ffmpeglib/lib/avcodec.lib \
        $$PWD/ffmpeglib/lib/avfilter.lib \
        $$PWD/ffmpeglib/lib/avformat.lib \
        $$PWD/ffmpeglib/lib/avutil.lib \
        $$PWD/ffmpeglib/lib/postproc.lib \
        $$PWD/ffmpeglib/lib/swresample.lib \
        $$PWD/ffmpeglib/lib/swscale.lib \
        $$PWD/SDL/lib/x64/SDL2.lib



SOURCES += \
    CAudio.cpp \
    CVideo.cpp \
    MyThread.cpp \
    main.cpp \
    Widget_Main.cpp

HEADERS += \
    CAudio.h \
    CVideo.h \
    MyThread.h \
    Widget_Main.h \
    ffmpeg.h

FORMS += \
    widget_main.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
