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

#include "circularbuffer.h"
#include "audiodevice.h"

using namespace Digital::Internal;

CircularBuffer::CircularBuffer(const QAudioFormat& format, qint64 samples, QObject* parent)
    : QIODevice(parent),
      m_format(format),
      m_bytesPerSample((format.sampleSize() / 8) * format.channelCount())
{
    m_readPtr = 0;
    m_writePtr = 0;
    m_bufferSize = 0;
    m_bufferCapacity = samples;

    m_buffer.resize(m_bufferCapacity);
    m_buffer.fill(0);
    open(QIODevice::ReadWrite);
}

CircularBuffer::~CircularBuffer()
{
    close();
}

void CircularBuffer::clear()
{
    m_readPtr = 0;
    m_writePtr = 0;
    m_bufferSize = 0;
}

qint64 CircularBuffer::readData(char* data, qint64 maxSize)
{
    qint64 bytesToRead = qMin(maxSize, m_bufferSize * m_bytesPerSample);

    qint64 bytesReadTotal = 0;
    qint64 samplesReadTotal = 0;

    qint64 bytesLeft = bytesToRead;
    while (bytesLeft > 0) {
        const double& value = m_buffer[m_readPtr];
        AudioDevice::realToPcm(m_format, value, data + bytesReadTotal);

        m_readPtr = (m_readPtr + 1) % m_buffer.size();
        bytesReadTotal += m_bytesPerSample;
        samplesReadTotal++;
        bytesLeft -= m_bytesPerSample;
    };

    m_bufferSize -= samplesReadTotal;
    if (m_bufferSize < 0)
        m_bufferSize = 0;

    emit bytesRead(bytesReadTotal);

    return bytesReadTotal;
}

qint64 CircularBuffer::writeData(const char* data, qint64 len)
{
    /*m_lock.lock();

    const int bytesPerSample = (m_format.sampleSize() / 8) * m_format.channelCount();

    qint64 bytesWrittenTotal = 0;
    qint64 samplesWrittenTotal = 0;

    // signal that a block of data is available as soon as the buffer is full
    qint64 bytesLeft = len;
    while (bytesLeft > 0) {
        qreal pcmSample = AudioDevice::pcmToReal(m_format, data + bytesWrittenTotal);
        m_buffer[m_position] = pcmSample;
        m_position = (m_position + 1) % m_buffer.size();

        bytesWrittenTotal += bytesPerSample;
        samplesWrittenTotal++;
        bytesLeft -= bytesPerSample;
    };

    m_bufferLength += samplesWrittenTotal;
    if (m_bufferLength > m_bufferSize)
        m_bufferLength = m_bufferSize;

    m_lock.unlock();

    emit bytesWritten(bytesWrittenTotal);

    return bytesWrittenTotal;*/
    return 0;
}


qint64 CircularBuffer::writeData(const double& sample)
{
    if (isFull())
        return 0;

    m_buffer[m_writePtr] = sample;
    m_writePtr = (m_writePtr + 1) % m_buffer.size();

    m_bufferSize += 1;
    if (m_bufferSize > m_bufferCapacity)
        m_bufferSize = m_bufferCapacity;

    return 1;
}

qint64 CircularBuffer::getBufferSize() const
{
    return m_bufferSize;
}

qint64 CircularBuffer::getBufferCapacity() const
{
    return m_bufferCapacity;
}

bool CircularBuffer::isEmpty() const
{
    return m_bufferSize == 0;
}

bool CircularBuffer::isFull() const
{
    return m_bufferSize == m_bufferCapacity;
}

qint64 CircularBuffer::getFreeSpace() const
{
    return m_bufferCapacity - m_bufferSize;
}
