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

#include "audioproducerlist.h"
#include "audioproducer.h"
#include "audiodeviceout.h"

#include <QDebug>

using namespace Digital::Internal;

AudioProducerList::AudioProducerList(AudioDeviceOut* device)
    : QIODevice(device),
      m_device(device)
{
}

AudioProducerList::~AudioProducerList()
{
}

bool AudioProducerList::add(AudioProducer* producer)
{
    if (!producer)
        return false;

    if (m_producerList.size() > 0) {
        qWarning() << "only one producer can be added at the moment";
        return false;
    }

    // check if the list already contains the item
    foreach (AudioProducer* p, m_producerList) {
        if (p == producer) {
            qWarning() << "producer is already registered";
            return false;
        }
    }

    // add the producer to the list
    connect(producer, &AudioProducer::newDataAvailable, this, &AudioProducerList::requestSoundcard);
    connect(producer, &AudioProducer::stopAudio, this, &AudioProducerList::stopSoundcard);
    m_producerList.push_back(producer);
    producer->registered();
    return true;
}

bool AudioProducerList::remove(AudioProducer* producer)
{
    if (!producer)
        return false;

    int count = m_producerList.removeAll(producer);

    if (count > 0) {
        producer->unregistered();
        return true;
    }

    return false;
}

void AudioProducerList::requestSoundcard()
{
    if (m_device && !m_device->isOpen()) {
        m_device->start();
    }
}

void AudioProducerList::stopSoundcard()
{
    if (m_device && m_device->isOpen()) {
        m_device->stop();
    }
}

qint64 AudioProducerList::writeData(const char* data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    qDebug() << "writing to audio consumer list is not supported";
    return 0;
}

qint64 AudioProducerList::readData(char* data, qint64 maxlen)
{
    if (maxlen == 0 || m_producerList.size() == 0)
        return 0;

    // read samples from the first producer
    AudioProducer* firstProducer = m_producerList[0];
    int bytesRead = firstProducer->read(data, maxlen);
    return bytesRead;
}
