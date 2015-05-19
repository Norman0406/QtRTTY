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

#include "modem.h"
#include "modemconfig.h"
#include "../signalprocessing/misc.h"
#include "modemfactory.h"
#include "modemtransmitter.h"
#include "modemreceiver.h"
#include "../audio/audiodevicein.h"
#include "../audio/audiodeviceout.h"
#include <QApplication>
#include <QDebug>

using namespace Digital::Internal;

Modem::Modem(unsigned capability, QObject* parent)
    : QObject(parent),
      m_capability(capability),
      m_frequency(1000),
      m_afcSpeed(AFC_NORMAL),
      m_freqErr(0),
      m_afc(true),
      m_reverse(false),
      m_internalState(INTSTATE_PREINIT),
      m_requestedState(INTSTATE_PREINIT),
      m_hasInputData(false),
      m_deviceIn(0),
      m_deviceOut(0),
      m_receiver(0),
      m_transmitter(0),
      m_thread(0),
      m_metric(0),
      m_nextCharacter(0),
      m_hasNextCharacter(false),
      m_autoMode(false)
{
    m_baseThread = thread();
}

Modem::~Modem()
{
    // NOTE: call shutdown() from derived classes

}

bool Modem::init(AudioDeviceIn* deviceIn, AudioDeviceOut* deviceOut)
{
    /*if (!deviceIn && hasCapability(CAP_RX))
        return false;

    if (!deviceOut && hasCapability(CAP_TX))
        return false;*/

    if ((!deviceIn && !deviceOut) ||
        (deviceIn && !deviceIn->isReady()) ||
        (deviceOut && !deviceOut->isReady()))
        return false;

    shutdown();

    // choose the format
    if (deviceIn && !deviceOut) {
        m_format = deviceIn->getFormat();
    }
    else if (deviceOut && !deviceIn) {
        m_format = deviceOut->getFormat();
    }
    else {
        if (deviceIn->getFormat() != deviceOut->getFormat())
            return false;

        m_format = deviceIn->getFormat();
    }

    if (!m_format.isValid())
        return false;

    m_deviceIn = deviceIn;
    m_deviceOut = deviceOut;

    if (m_deviceOut && hasCapability(CAP_TX)) {
        // create a buffer that has a resolution of 100 ms
        qint32 bufferSize = (m_format.sampleRate() / m_format.channelCount()) * 0.1;

        m_transmitter = new ModemTransmitter(this, bufferSize);
        m_deviceOut->registerProducer(m_transmitter);
    }

    if (m_deviceIn && hasCapability(CAP_RX)) {
        m_receiver = new ModemReceiver(this, this);
        m_deviceIn->registerConsumer(m_receiver);
    }

    if (!iInit())
        return false;

    restart();

    m_thread = new QThread(parent());
    setParent(0);
    connect(m_thread, &QThread::started, this, &Modem::process);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    moveToThread(m_thread);
    m_thread->start();

    // wait until thread started
    QMutexLocker lock(&m_stateChangedMutex);
    while (m_internalState != INTSTATE_READY)
        m_stateChangedCond.wait(&m_stateChangedMutex);

    emit initialized(true);

    return true;
}

void Modem::restart()
{
    m_freqErr = 0.0;
    m_metric = 0.0;
    m_hasInputData = false;
    m_nextCharacter = 0;
    m_hasNextCharacter = false;
    m_autoMode = false;

    iRestart();
}

void Modem::shutdown()
{
    if (m_internalState == INTSTATE_PREINIT)
        return;

    if (isTransmitting())
        stopTx();

    if (isReceiving())
        stopRx();

    setInternalState(INTSTATE_SHUTDOWN);

    m_waitForData.wakeAll();
    m_thread->quit();
    m_thread->wait();

    QObject* baseParent = m_thread->parent();

    delete m_thread;
    m_thread = 0;

    moveToThread(m_baseThread);
    setParent(baseParent);

    if (m_receiver) {
        if (m_deviceIn)
            m_deviceIn->unregisterConsumer(m_receiver);
        delete m_receiver;
        m_receiver = 0;
    }

    if (m_transmitter) {
        if (m_deviceOut)
            m_deviceOut->unregisterProducer(m_transmitter);
        delete m_transmitter;
        m_transmitter = 0;
    }

    iShutdown();
    setInternalState(INTSTATE_PREINIT);

    emit initialized(false);
}

void Modem::setFrequency(double frequency)
{
    m_frequency = frequency;
    emit frequencyChanged(m_frequency);
}

void Modem::setAFCSpeed(AFCSpeed afcSpeed)
{
    m_afcSpeed = afcSpeed;
}

bool Modem::hasCapability(Capability cap) const
{
    return m_capability & cap;
}

double Modem::getFrequency() const
{
    return m_frequency;
}

double Modem::getMetric() const
{
    return m_metric;
}

Modem::AFCSpeed Modem::getAFCSpeed() const
{
    return m_afcSpeed;
}

int Modem::getSampleRate() const
{
    return m_format.sampleRate();
}

void Modem::adjustFrequency(double frqerr)
{
    m_freqErr = decayAvg(m_freqErr, frqerr / 8, m_afcSpeed == AFC_SLOW ? 8 : m_afcSpeed == AFC_NORMAL ? 4 : 1);

    if (m_afc) {
        setFrequency(m_frequency - m_freqErr);
    }
}

void Modem::process()
{
    // signal that the thread started running
    setInternalState(INTSTATE_READY);

    while (m_internalState != INTSTATE_SHUTDOWN) {
        QCoreApplication::processEvents();

        QMutexLocker lock(&m_waitMutex);
        if (m_requestedState != m_internalState)
            setInternalState(m_requestedState);

        while (m_internalState == INTSTATE_PREINIT || m_internalState == INTSTATE_READY ||
               (m_internalState == INTSTATE_RX && !m_hasInputData))
            m_waitForData.wait(&m_waitMutex);

        switch (m_internalState) {
        case INTSTATE_RX:
            iRxProcess(m_inputBlock);

            m_metric = computeMetric();

            m_hasInputData = false;
            break;
        case INTSTATE_TX_STARTING:
        case INTSTATE_TX_STOPPING:
        case INTSTATE_TX:
            if (m_internalState == INTSTATE_TX_STARTING)
                m_transmitter->start();

            iTxProcess();

            if (m_internalState == INTSTATE_TX_STARTING) {
                if (m_requestedState == m_internalState)
                    setInternalState(INTSTATE_TX);
            }
            else if (m_internalState == INTSTATE_TX_STOPPING) {
                m_transmitter->stop();
                setInternalState(INTSTATE_RX);

                m_txStoppedCond.wakeAll();
            }

            break;
        default:
            break;
        }
    }
}

bool Modem::startTx()
{
    if (m_transmitter && hasCapability(CAP_TX) && !isTransmitting()) {
        QMutexLocker lock(&m_waitMutex);
        m_autoMode = false;
        setInternalState(INTSTATE_TX_STARTING);
        m_waitForData.wakeAll();

        return true;
    }

    return false;
}

bool Modem::startTxAuto()
{
    if (m_transmitter && hasCapability(CAP_TX) && !isTransmitting()) {
        QMutexLocker lock(&m_waitMutex);
        m_autoMode = true;
        setInternalState(INTSTATE_TX_STARTING);
        m_waitForData.wakeAll();

        return true;
    }

    return false;
}

bool Modem::setNextCharacter(char character)
{
    /*if (!isTransmitting() || m_hasNextCharacter)
        return false;*/

    if (m_hasNextCharacter)
        return false;

    QMutexLocker lock(&m_nextCharacterMutex);
    m_nextCharacter = (int)character;
    m_hasNextCharacter = true;

    return true;
}

bool Modem::clearNextCharacter()
{
    /*if (!isTransmitting())
        return false;*/

    QMutexLocker lock(&m_nextCharacterMutex);
    m_hasNextCharacter = false;

    return true;
}

bool Modem::stopTx()
{
    if (m_transmitter && hasCapability(CAP_TX) && isTransmitting()) {
        m_requestedState = INTSTATE_TX_STOPPING;

        // wait until tx stopped
        QMutexLocker lock(&m_waitMutex);
        while (isTransmitting())
            m_txStoppedCond.wait(&m_waitMutex);

        return true;
    }

    return false;
}

bool Modem::isTransmitting() const
{
    return m_internalState == INTSTATE_TX_STARTING || m_internalState == INTSTATE_TX || m_internalState == INTSTATE_TX_STOPPING;
}

bool Modem::startRx()
{
    if (m_receiver && hasCapability(CAP_RX)) {
        QMutexLocker lock(&m_waitMutex);
        m_receiver->start();
        setInternalState(INTSTATE_RX);
        m_waitForData.wakeAll();

        return true;
    }

    return false;
}

void Modem::receive(const QVector<double>& data)
{
    QMutexLocker lock(&m_waitMutex);
    if (m_internalState == INTSTATE_RX) {
        if (m_hasInputData)
            return;

        m_inputBlock = data;
        m_waitForData.wakeAll();
        m_hasInputData = true;
    }
}

bool Modem::stopRx()
{
    if (m_receiver && hasCapability(CAP_RX) && isReceiving()) {
        QMutexLocker lock(&m_waitMutex);
        m_receiver->start();
        setInternalState(INTSTATE_READY);
        m_waitForData.wakeAll();

        return true;
    }

    return false;
}

bool Modem::isReceiving() const
{
    return m_internalState == INTSTATE_RX;
}

bool Modem::getNextChar(QChar& c)
{
    if (m_autoMode && !m_hasNextCharacter) {
        m_requestedState = INTSTATE_TX_STOPPING;
        return false;
    }

    emit requestNextCharacter();

    QMutexLocker lock(&m_nextCharacterMutex);
    if (m_hasNextCharacter) {
        c = QChar(m_nextCharacter);
        m_hasNextCharacter = false;

        return true;
    }

    return false;
}

bool Modem::writeSample(double sample)
{
    if (!isTransmitting()|| !m_transmitter)
        return false;

    m_transmitter->writeValue(sample);

    return true;
}

double Modem::getFrqErr() const
{
    return m_freqErr;
}

bool Modem::setAFC(bool enabled)
{
    if (hasCapability(CAP_AFC)) {
        m_afc = enabled;
        return true;
    }
    return false;
}

bool Modem::setReverse(bool enabled)
{
    if (hasCapability(CAP_REV)) {
        m_reverse = enabled;
        return true;
    }
    return false;
}

bool Modem::getAFC() const
{
    return m_afc;
}

bool Modem::isReverse() const
{
    return m_reverse;
}

Modem::InternalState Modem::getInternalState() const
{
    return m_internalState;
}

void Modem::setInternalState(Modem::InternalState state)
{
    m_stateChangedMutex.lock();

    m_internalState = state;
    m_requestedState = state;

    m_stateChangedCond.wakeAll();
    m_stateChangedMutex.unlock();
}

double Modem::isSquelchOpen() const
{
    return true;
}
