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

#include "audiodeviceinthread.h"

#include <QDebug>

using namespace Digital::Internal;

AudioDeviceInThread::AudioDeviceInThread(QAudioDeviceInfo deviceInfo, QAudioFormat format, QIODevice* ioDevice)
    : m_audioInput(0),
      m_format(format),
      m_deviceInfo(deviceInfo),
      m_ioDevice(ioDevice),
      m_isStarted(false)
{
}

AudioDeviceInThread::~AudioDeviceInThread()
{
    quit();
    wait();
    if (m_audioInput)
        m_audioInput->stop();
}

bool AudioDeviceInThread::startAudio()
{
    if (m_isStarted && getState() != QAudio::ActiveState) {
        if (!m_ioDevice->isOpen())
            m_ioDevice->open(QIODevice::WriteOnly);

        m_audioInput->start(m_ioDevice);
        return true;
    }

    return false;
}

void AudioDeviceInThread::stopAudio()
{
    if (m_isStarted) {
        m_audioInput->stop();

        if (m_ioDevice->isOpen())
            m_ioDevice->close();
    }
}

QAudio::State AudioDeviceInThread::getState() const
{
    if (m_audioInput)
        return m_audioInput->state();

    return QAudio::StoppedState;
}

int AudioDeviceInThread::getPeriodSize() const
{
    if (m_audioInput)
        return m_audioInput->periodSize();

    return -1;
}

int AudioDeviceInThread::getBufferSize() const
{
    if (m_audioInput)
        return m_audioInput->bufferSize();

    return -1;
}

bool AudioDeviceInThread::isStarted() const
{
    return m_isStarted;
}

void AudioDeviceInThread::startThread()
{
    start();
    QObject::moveToThread(this);

    QMutexLocker lock(&m_initMutex);
    while (!m_isStarted)
        m_initCond.wait(&m_initMutex);
}

void AudioDeviceInThread::run()
{
    m_audioInput = new QAudioInput(m_deviceInfo, m_format);
    connect(m_audioInput, &QAudioInput::stateChanged, this, &AudioDeviceInThread::stateChanged);

    QMutexLocker lock(&m_initMutex);
    m_isStarted = true;
    m_initCond.wakeAll();
    lock.unlock();

    exec();
}
