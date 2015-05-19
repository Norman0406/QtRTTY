/***********************************************************************
 *
 * LISA: Lightweight Integrated System for Amateur Radio
 * Copyright (C) 2013 - 2014
 *      Norman Link (DM6LN)
 *
 * This file is part of LISA.
 *
 * LISA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LISA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You can find a copy of the GNU General Public License in the file
 * LICENSE.GPL contained in the root directory of this project or
 * under <http://www.gnu.org/licenses/>.
 *
 **********************************************************************/

#ifndef AUDIODEVICEOUTTHREAD_H
#define AUDIODEVICEOUTTHREAD_H

#include "audiodeviceout.h"
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>

namespace Digital {
namespace Internal {

class AudioDeviceOutThread
        : public QThread
{
    Q_OBJECT

    friend class AudioDeviceOut;

private slots:
    bool            startAudio();
    void            stopAudio();
    QAudio::State   getState() const;
    int             getPeriodSize() const;
    int             getBufferSize() const;
    bool            isStarted() const;
    void            startThread();

signals:
    void            stateChanged(QAudio::State state);

private:
    AudioDeviceOutThread(QAudioDeviceInfo deviceInfo, QAudioFormat format, QIODevice* ioDevice);
    ~AudioDeviceOutThread();

    void run();

    QMutex              m_initMutex;
    QWaitCondition      m_initCond;

    QAudioOutput*       m_audioOutput;
    QAudioFormat        m_format;
    QAudioDeviceInfo    m_deviceInfo;
    QIODevice*          m_ioDevice;
    bool                m_isStarted;
};

} // namespace Internal
} // namespace Digital

#endif // AUDIODEVICEOUTTHREAD_H
