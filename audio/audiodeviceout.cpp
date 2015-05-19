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

#include "audiodeviceout.h"
#include "audiodeviceoutthread.h"
#include "audioproducerlist.h"
#include "audioproducer.h"
#include <QDebug>
#include <QAudioDeviceInfo>
#include <QtEndian>

using namespace Digital::Internal;

AudioDeviceOut::AudioDeviceOut(QObject* parent, QAudioDeviceInfo deviceInfo)
    : AudioDevice(parent, deviceInfo),
      m_thread(0)
{
    m_producerList = new AudioProducerList(this);
}

AudioDeviceOut::~AudioDeviceOut()
{
    close();
    delete m_producerList;
}

bool AudioDeviceOut::iInit(const QAudioDeviceInfo& info)
{
    close();

    m_thread = new AudioDeviceOutThread(info, getFormat(), m_producerList);
    connect(this, &AudioDeviceOut::startAudio, m_thread, &AudioDeviceOutThread::startAudio);
    connect(this, &AudioDeviceOut::stopAudio, m_thread, &AudioDeviceOutThread::stopAudio);
    connect(m_thread, &AudioDeviceOutThread::stateChanged, this, &AudioDeviceOut::stateChanged);
    m_thread->startThread();

    return true;
}

bool AudioDeviceOut::iClose()
{
    if (m_thread) {
        delete m_thread;
        m_thread = 0;
    }

    return true;
}

bool AudioDeviceOut::start()
{
    if (m_thread && m_thread->isStarted()) {
        emit startAudio();
        return true;
    }

    return false;
}

bool AudioDeviceOut::stop()
{
    if (m_thread && m_thread->isStarted()) {
        emit stopAudio();
        return true;
    }

    return false;
}

bool AudioDeviceOut::isOpen() const
{
    if (m_thread)
        return m_thread->getState() == QAudio::ActiveState;

    return false;
}

qint32 AudioDeviceOut::getBufferSize() const
{
    if (m_thread)
        return m_thread->getBufferSize();

    return -1;
}

bool AudioDeviceOut::registerProducer(AudioProducer* producer)
{
    if (m_producerList->add(producer)) {
        producer->create(getFormat());
        return true;
    }

    return false;
}

bool AudioDeviceOut::unregisterProducer(AudioProducer* producer)
{
    return m_producerList->remove(producer);
}

void AudioDeviceOut::stateChanged(QAudio::State state)
{
    qDebug() << "out state changed: " << state;

    switch (state) {
    case QAudio::ActiveState:
        break;
    case QAudio::SuspendedState:
        break;
    case QAudio::StoppedState:
        break;
    case QAudio::IdleState:
        stop(); // TODO: may be stop it directly?
        break;
    }
}
