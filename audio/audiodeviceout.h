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

#ifndef AUDIODEVICEOUT_H
#define AUDIODEVICEOUT_H

#include "audiodevice.h"
#include <QMutex>
#include <QWaitCondition>
#include <QAudioDeviceInfo>

namespace Digital {
namespace Internal {

class AudioProducer;
class AudioProducerList;
class AudioDeviceOutThread;

class AudioDeviceOut
        : public AudioDevice
{
    Q_OBJECT

public:
    AudioDeviceOut(QObject* parent, QAudioDeviceInfo deviceInfo);
    ~AudioDeviceOut();

    bool start();
    bool stop();
    bool isOpen() const;
    virtual int getBufferSize() const;

    bool registerProducer(AudioProducer*);
    bool unregisterProducer(AudioProducer*);

signals:
    void startAudio();
    void stopAudio();

private slots:
    void stateChanged(QAudio::State);

protected:
    bool iInit(const QAudioDeviceInfo&);
    bool iClose();

private:
    QMutex                  m_initMutex;
    QWaitCondition          m_initCond;
    QAudioDeviceInfo        m_info;
    AudioDeviceOutThread*   m_thread;
    AudioProducerList*      m_producerList;
};

} // namespace Internal
} // namespace Digital

#endif // AUDIODEVICEOUT_H
