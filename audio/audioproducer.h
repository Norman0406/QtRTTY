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

#ifndef AUDIOPRODUCER_H
#define AUDIOPRODUCER_H

#include <QObject>
#include <QThread>
#include "circularbuffer.h"

namespace Digital {
namespace Internal {

class AudioProducerList;

class AudioProducer
        : public QObject
{
    Q_OBJECT

    friend class AudioProducerList;

public:
    AudioProducer(QObject* parent, qint32);
    ~AudioProducer();

    virtual void create(QAudioFormat);

    qint64 read(char* data, qint64 maxlen);

    virtual void start();
    virtual void stop();

signals:
    void newDataAvailable();
    void stopAudio();

protected:
    void write(const double& sample);

    const QAudioFormat& getFormat() const;

    virtual void registered();
    virtual void unregistered();

private:
    qint64 m_dataRequested;
    qint64 m_dataGenerated;
    QWaitCondition m_dataWaitCond;
    QMutex m_dataMutex;
    QWaitCondition m_stopCond;

    int m_bytesPerSample;
    QAudioFormat m_format;
    qint32 m_bufferSize;
    QMutex m_bufferMutex;
    QWaitCondition m_waitCond;
    CircularBuffer* m_buffer;
    bool m_terminate;
};

} // namespace Internal
} // namespace Digital

#endif // AUDIOPRODUCER_H
