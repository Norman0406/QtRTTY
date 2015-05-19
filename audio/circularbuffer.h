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

#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <QIODevice>
#include <QAudioFormat>
#include <QMutex>
#include <QVector>
#include <QWaitCondition>

namespace Digital {
namespace Internal {

// TODO: implement multiple channels
class CircularBuffer
        : public QIODevice
{
    Q_OBJECT

public:
    CircularBuffer(const QAudioFormat& format, qint64 samples, QObject*);
    ~CircularBuffer();

    void clear();

    qint64 readData(char*, qint64);
    qint64 writeData(const char*, qint64);
    qint64 writeData(const double& sample);

    qint64 getBufferSize() const;
    qint64 getBufferCapacity() const;
    qint64 getFreeSpace() const;

    bool isEmpty() const;
    bool isFull() const;

signals:
    void bytesRead(qint64);
    void bytesWritten(qint64);

private:
    qint64 m_readPtr;
    qint64 m_writePtr;
    qint64 m_bufferSize;
    qint64 m_bufferCapacity;
    QVector<double> m_buffer;
    const QAudioFormat m_format;
    const int m_bytesPerSample;
};

} // namespace Internal
} // namespace Digital

#endif // CIRCULARBUFFER_H
