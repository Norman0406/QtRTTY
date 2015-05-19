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

#include "audiodevicein.h"
#include "audiodeviceinthread.h"
#include "audioconsumerlist.h"
#include "audioconsumer.h"
#include <QDebug>
#include <QAudioDeviceInfo>
#include <QtEndian>

using namespace Digital::Internal;

AudioDeviceIn::AudioDeviceIn(QObject* parent, QAudioDeviceInfo deviceInfo)
    : AudioDevice(parent, deviceInfo),
    m_thread(0)
{
    m_consumerList = new AudioConsumerList(this);
}

AudioDeviceIn::~AudioDeviceIn()
{
    close();
    delete m_consumerList;
}

bool AudioDeviceIn::iInit(const QAudioDeviceInfo& info)
{
    close();

    m_thread = new AudioDeviceInThread(info, getFormat(), m_consumerList);
    connect(this, &AudioDeviceIn::startAudio, m_thread, &AudioDeviceInThread::startAudio);
    connect(this, &AudioDeviceIn::stopAudio, m_thread, &AudioDeviceInThread::stopAudio);
    connect(m_thread, &AudioDeviceInThread::stateChanged, this, &AudioDeviceIn::stateChanged);
    m_thread->startThread();

    return true;
}

bool AudioDeviceIn::iClose()
{
    if (m_thread) {
        delete m_thread;
        m_thread = 0;
    }

    return true;
}

bool AudioDeviceIn::start()
{
    if (m_thread && m_thread->isStarted()) {
        emit startAudio();
        return true;
    }

    return false;
}

bool AudioDeviceIn::stop()
{
    if (m_thread && m_thread->isStarted()) {
        emit stopAudio();
        return true;
    }

    return false;
}

bool AudioDeviceIn::isOpen() const
{
    if (m_thread)
        return m_thread->getState() == QAudio::ActiveState;

    return false;
}

qint32 AudioDeviceIn::getBufferSize() const
{
    if (m_thread)
        return m_thread->getBufferSize();

    return -1;
}

bool AudioDeviceIn::registerConsumer(AudioConsumer* consumer)
{
    if (m_consumerList->add(consumer)) {
        consumer->create(getFormat());

        //connect(consumer, SIGNAL(destroyed()), this, SLOT(unregisterConsumer(AudioConsumer*)));

        return true;
    }

    return false;
}

bool AudioDeviceIn::unregisterConsumer(AudioConsumer* consumer)
{
    return m_consumerList->remove(consumer);
}

void AudioDeviceIn::stateChanged(QAudio::State state)
{
    qDebug() << "in state changed: " << state;

    switch (state) {
    case QAudio::ActiveState:
        break;
    case QAudio::SuspendedState:
        break;
    case QAudio::StoppedState:
        break;
    case QAudio::IdleState:
        break;
    }
}
