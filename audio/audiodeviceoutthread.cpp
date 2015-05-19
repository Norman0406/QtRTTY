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

#include "audiodeviceoutthread.h"

#include <QDebug>

using namespace Digital::Internal;

AudioDeviceOutThread::AudioDeviceOutThread(QAudioDeviceInfo deviceInfo, QAudioFormat format, QIODevice* ioDevice)
    : m_audioOutput(0),
      m_format(format),
      m_deviceInfo(deviceInfo),
      m_ioDevice(ioDevice),
      m_isStarted(false)
{
}

AudioDeviceOutThread::~AudioDeviceOutThread()
{
    quit();
    wait();
    if (m_audioOutput)
        m_audioOutput->stop();
}

bool AudioDeviceOutThread::startAudio()
{
    if (m_isStarted && getState() != QAudio::ActiveState) {
        if (!m_ioDevice->isOpen())
            m_ioDevice->open(QIODevice::ReadOnly);

        m_audioOutput->start(m_ioDevice);
        return true;
    }

    return false;
}

void AudioDeviceOutThread::stopAudio()
{
    if (m_isStarted) {
        if (m_ioDevice->isOpen())
            m_ioDevice->close();

        m_audioOutput->stop();
    }
}

QAudio::State AudioDeviceOutThread::getState() const
{
    if (m_audioOutput)
        return m_audioOutput->state();

    return QAudio::StoppedState;
}

int AudioDeviceOutThread::getPeriodSize() const
{
    if (m_audioOutput)
        return m_audioOutput->periodSize();

    return -1;
}

int AudioDeviceOutThread::getBufferSize() const
{
    if (m_audioOutput)
        return m_audioOutput->bufferSize();

    return -1;
}

bool AudioDeviceOutThread::isStarted() const
{
    return m_isStarted;
}

void AudioDeviceOutThread::startThread()
{
    start();
    QObject::moveToThread(this);

    QMutexLocker lock(&m_initMutex);
    while (!m_isStarted)
        m_initCond.wait(&m_initMutex);
}

void AudioDeviceOutThread::run()
{
    m_audioOutput = new QAudioOutput(m_deviceInfo, m_format);
    connect(m_audioOutput, &QAudioOutput::stateChanged, this, &AudioDeviceOutThread::stateChanged);

    QMutexLocker lock(&m_initMutex);
    m_isStarted = true;
    m_initCond.wakeAll();
    lock.unlock();

    exec();
}
