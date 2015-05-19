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

#include "audioconsumer.h"
#include "audiodevice.h"
#include <QDebug>

using namespace Digital::Internal;

AudioConsumer::AudioConsumer(QObject* parent, qint64 samples)
    : QObject(0),
      m_samples(samples),
      m_thread(0),
      m_terminate(false),
      m_dataReady(false),
      m_position(0)
{
    setNumSamples(samples);

    m_thread = new QThread(parent);
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, &AudioConsumer::processThread);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
}

AudioConsumer::~AudioConsumer()
{
    m_waitMutex.lock();
    m_terminate = true;
    m_waitCond.wakeAll();
    m_waitMutex.unlock();
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}

void AudioConsumer::setNumSamples(qint64 samples)
{
    m_waitMutex.lock();
    m_samples = samples;
    m_position = 0;
    m_buffer.resize(m_samples);
    m_buffer.fill(0);
    m_data.resize(m_samples);
    m_data.fill(0);
    m_waitMutex.unlock();
}

qint64 AudioConsumer::getNumSamples() const
{
    return m_samples;
}

void AudioConsumer::create(QAudioFormat format)
{
    m_format = format;
}

const QAudioFormat& AudioConsumer::getFormat() const
{
    return m_format;
}

qint64 AudioConsumer::writeData(const char* data, qint64 len)
{
    const int bytesPerSample = (getFormat().sampleSize() / 8) * getFormat().channelCount();

    if (getFormat().isValid()) {
        qint64 bytesWrittenTotal = 0;

        // signal that a block of data is available as soon as the buffer is full
        qint64 bytesLeft = len;
        do {
            qreal pcmSample = AudioDevice::pcmToReal(getFormat(), data + bytesWrittenTotal);
            m_buffer[m_position] = pcmSample;
            m_position++;

            // the buffer is full, convert the data block into real data
            if (m_position == m_buffer.size()) {
                m_position = 0;

                // If a complete chunk of data has been written, see if the worker thread can already process new data and
                // copy from m_buffer to m_data. If the worker thread is still computing, don't copy data and continue. This will
                // overwrite previous data.
                if (m_waitMutex.tryLock()) {
                    // copy data from buffer
                    memcpy((void*)m_data.data(), (void*)m_buffer.data(), m_buffer.size() * sizeof(double));
                    m_dataReady = true;
                    m_waitMutex.unlock();

                    // signal that new data is available
                    m_waitCond.wakeAll();
                }
            }

            bytesWrittenTotal += bytesPerSample;
            bytesLeft -= bytesPerSample;
        } while (bytesLeft > 0);

        return bytesWrittenTotal;
    }

    return 0;
}

void AudioConsumer::processThread()
{
    forever {
        QMutexLocker lock(&m_waitMutex);
        while (!m_dataReady && !m_terminate)
            m_waitCond.wait(&m_waitMutex);

        if (m_terminate)
            break;

        // process audio data
        processAudio(m_data);

        m_dataReady = false;
        lock.unlock();
    }
}

void AudioConsumer::start()
{
    emit startAudio();
}

void AudioConsumer::stop()
{
    emit stopAudio();
}

void AudioConsumer::registered()
{
}

void AudioConsumer::unregistered()
{
}
