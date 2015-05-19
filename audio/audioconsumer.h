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

#ifndef AUDIOCONSUMER_H
#define AUDIOCONSUMER_H

#include <QObject>
#include <QAudioFormat>
#include <QBuffer>
#include <QVector>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>

namespace Digital {
namespace Internal {

class AudioConsumerList;

/**
 * @brief The AudioConsumer class provides the main interface between any audio
 * input consumer and the actual audio device. Any module that needs to process
 * input audio data is a subclass of AudioConsumer and registers itself to the
 * input device.
 */
class AudioConsumer
        : public QObject
{
    Q_OBJECT

    friend class AudioConsumerList;

public:
    ~AudioConsumer();

    virtual void create(QAudioFormat);

    void setNumSamples(qint64);
    qint64 getNumSamples() const;

    qint64 writeData(const char* data, qint64 len);

    virtual void start();
    virtual void stop();

signals:
    void startAudio();
    void stopAudio();

protected:
    AudioConsumer(QObject* parent, qint64 samples);
    const QAudioFormat& getFormat() const;

    virtual void registered();
    virtual void unregistered();

    /**
     * @brief Is called in a separate thread when the next block of continous frames is available
     * @param data the audio data block containing m_frames audio frames
     */
    virtual void processAudio(const QVector<double>& data) = 0;

private slots:
    void processThread();

private:
    QAudioFormat    m_format;
    qint64          m_samples;
    QWaitCondition  m_waitCond;
    QMutex          m_waitMutex;
    QThread*        m_thread;
    bool            m_terminate;
    bool            m_dataReady;
    int             m_position;
    QVector<double> m_data;     // the data that is sent to the consumer
    QVector<double> m_buffer;   // the buffer that gets written to
};

} // namespace Internal
} // namespace Digital

#endif // AUDIOCONSUMER_H
