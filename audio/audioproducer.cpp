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

#include "audioproducer.h"
#include "audioproducerlist.h"
#include <QDebug>

using namespace Digital::Internal;

AudioProducer::AudioProducer(QObject* parent, qint32 bufferSize)
    : QObject(parent),
      m_bufferSize(bufferSize),
      m_buffer(0),
      m_terminate(false),
      m_dataRequested(0),
      m_dataGenerated(0),
      m_bytesPerSample(0)
{
}

AudioProducer::~AudioProducer()
{
    delete m_buffer;
}

void AudioProducer::create(QAudioFormat format)
{
    if (m_buffer)
        return;

    m_format = format;
    m_bytesPerSample = (m_format.sampleSize() / 8) * m_format.channelCount();
    m_buffer = new CircularBuffer(m_format, m_bufferSize, this);
}

qint64 AudioProducer::read(char* data, qint64 maxlen)
{
    if (m_terminate)
        return 0;

    // request samples and wait until the buffer is sufficiently filled
    QMutexLocker lock(&m_bufferMutex);

    // the number of samples that are requested (maxlen is in bytes while m_buffer->getBufferCapacity() is in # samples)
    m_dataRequested = qMin(maxlen / m_bytesPerSample, m_buffer->getBufferCapacity());

    // wait until data is available in the buffer
    m_dataWaitCond.wakeAll();
    while (m_dataGenerated == 0 && !m_terminate)
        m_waitCond.wait(&m_bufferMutex);

    // read data from the buffer
    qint64 dataRead = 0;
    if (m_dataGenerated > 0)
        dataRead = m_buffer->read(data, m_dataGenerated * m_bytesPerSample);
    m_dataGenerated = 0;
    m_stopCond.wakeAll();
    return dataRead;
}

void AudioProducer::write(const double& sample)
{
    // wait until a number of samples has been requested
    QMutexLocker lock(&m_bufferMutex);
    while (m_dataRequested == 0)
        m_dataWaitCond.wait(&m_bufferMutex);

    if (sample < -1 || sample > 1)
        qWarning() << "samples are out of range";

    // if there is enough space in the buffer, write the sample to the buffer
    m_buffer->writeData(sample);

    // if the buffer is filled sufficiently, signal that data can be read
    if (m_buffer->getBufferSize() >= m_dataRequested) {
        m_dataGenerated = m_buffer->getBufferSize();
        m_waitCond.wakeAll();
        m_dataRequested = 0;
    }
}

void AudioProducer::start()
{
    m_dataRequested = 0;
    m_terminate = false;
    emit newDataAvailable();
}

void AudioProducer::stop()
{
    QMutexLocker lock(&m_bufferMutex);

    // request to read all remaining data in the buffer
    m_terminate = true;
    m_dataGenerated = m_buffer->getBufferSize();
    m_waitCond.wakeAll();

    // wait until the buffer is empty, then stop the audio
    while (m_buffer->getBufferSize() > 0)
        m_stopCond.wait(&m_bufferMutex);

    emit stopAudio();
}

const QAudioFormat& AudioProducer::getFormat() const
{
    return m_format;
}

void AudioProducer::registered()
{
    // not implemented
}

void AudioProducer::unregistered()
{
    // not implemented
}
