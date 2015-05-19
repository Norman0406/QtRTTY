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

#include "audioconsumerlist.h"
#include "audioconsumer.h"
#include "audiodevicein.h"

#include <QDebug>

using namespace Digital::Internal;

AudioConsumerList::AudioConsumerList(AudioDeviceIn* device)
    : QIODevice(device),
      m_device(device)
{
}

AudioConsumerList::~AudioConsumerList()
{
}

bool AudioConsumerList::add(AudioConsumer* consumer)
{
    if (!consumer)
        return false;

    // register new audio consumer
    connect(consumer, &AudioConsumer::startAudio, this, &AudioConsumerList::requestSoundcard);
    connect(consumer, &AudioConsumer::stopAudio, this, &AudioConsumerList::stopSoundcard);
    m_consumerList.push_back(consumer);
    consumer->registered();

    return true;
}

bool AudioConsumerList::remove(AudioConsumer* consumer)
{
    if (!consumer)
        return false;

    int count = m_consumerList.removeAll(consumer);

    if (count > 0) {
        consumer->unregistered();
        return true;
    }

    return false;
}

void AudioConsumerList::requestSoundcard()
{
    // TODO: open soundcard only if all other consumers are not running

    if (m_device && !m_device->isOpen()) {
        m_device->start();
    }
}

void AudioConsumerList::stopSoundcard()
{
    // TODO: stop soundcard only if all consumers have been stopped

    if (m_device && m_device->isOpen()) {
        m_device->stop();
    }
}

qint64 AudioConsumerList::writeData(const char* data, qint64 len)
{
    // write data into each registered audio consumer
    qint64 bytesWritten = 0;
    foreach (AudioConsumer* consumer, m_consumerList) {
        qint64 written = consumer->writeData(data, len);
        bytesWritten = qMax(bytesWritten, written);
    }

    return bytesWritten;
}

qint64 AudioConsumerList::readData(char* data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    qDebug() << "reading from audio consumer list is not supported";
    return 0;
}
