#-------------------------------------------------
#
# Project created by QtCreator 2014-07-14T23:29:05
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AudioMixer
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    modems/modem.cpp \
    modems/modemconfig.cpp \
    modems/modemrtty.cpp \
    modems/modemrttyconfig.cpp \
    modems/modemtransmitter.cpp \
    signalprocessing/fftfilter.cpp \
    signalprocessing/fftspectrum.cpp \
    signalprocessing/fftspectrumworker.cpp \
    signalprocessing/filters.cpp \
    signalprocessing/misc.cpp \
    audio/audioconsumer.cpp \
    audio/audioconsumerlist.cpp \
    audio/audiodevice.cpp \
    audio/audiodevicein.cpp \
    audio/audiodeviceinthread.cpp \
    audio/audiodevicelist.cpp \
    audio/audiodeviceout.cpp \
    audio/audiodeviceoutthread.cpp \
    audio/audioproducer.cpp \
    audio/audioproducerlist.cpp \
    audio/audioringbuffer.cpp \
    audio/circularbuffer.cpp \
    modems/modemreceiver.cpp \
    transmittertextedit.cpp

HEADERS  += mainwindow.h \
    factory.h \
    modems/modem.h \
    modems/modemconfig.h \
    modems/modemfactory.h \
    modems/modemrtty.h \
    modems/modemrttyconfig.h \
    modems/modemtransmitter.h \
    signalprocessing/fftfilter.h \
    signalprocessing/fftspectrum.h \
    signalprocessing/fftspectrumworker.h \
    signalprocessing/filters.h \
    signalprocessing/misc.h \
    audio/audioconsumer.h \
    audio/audioconsumerlist.h \
    audio/audiodevice.h \
    audio/audiodevicein.h \
    audio/audiodeviceinthread.h \
    audio/audiodevicelist.h \
    audio/audiodeviceout.h \
    audio/audiodeviceoutthread.h \
    audio/audioproducer.h \
    audio/audioproducerlist.h \
    audio/audioringbuffer.h \
    audio/circularbuffer.h \
    modems/modemreceiver.h \
    transmittertextedit.h

FORMS    += mainwindow.ui

INCLUDEPATH += external/include

LIBS += $$_PRO_FILE_PWD_/external/bin/libfftw3-3.dll

#DEFINES += _USE_MATH_DEFINES
