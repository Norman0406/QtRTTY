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

#include "modemrtty.h"
#include "modemworker.h"
#include "modemreceiver.h"
#include "modemtransmitter.h"
#include "../factory.h"
#include "../audio/audiodevicein.h"
#include "../audio/audiodeviceout.h"

#include <QApplication>
#include <QDebug>

using namespace Digital::Internal;

ModemWorker::ModemWorker()
    : QObject(0),
      m_modem(0),
      m_receiver(0),
      m_transmitter(0),
      m_terminate(false),
      m_isBufferEmpty(true)
{
}

ModemWorker::~ModemWorker()
{
    terminate();
    delete m_modem;
}

void ModemWorker::initReceiver(AudioDeviceIn* inputDevice)
{
    if (!m_receiver) {
        //m_receiver = new ModemReceiver(this);
        inputDevice->registerConsumer(m_receiver);
        init(inputDevice->getFormat());
    }
    else {
        // TODO: reset receiver
    }
}

void ModemWorker::initTransmitter(AudioDeviceOut* outputDevice)
{
    if (!m_transmitter) {
        m_transmitter = new ModemTransmitter(this);
        outputDevice->registerProducer(m_transmitter);
    }
    else {
        // TODO: reset transmitter
    }
}

bool ModemWorker::create(QString type)
{
    if (m_modem)
        return false;

    m_modem = Factory<Modem>::createByType(type, this);
    return true;
}

bool ModemWorker::init(const QAudioFormat& format)
{
    if (m_modem && m_modem->getState() == Modem::STATE_PREINIT) {
        if (m_modem->init(format)) {
            connect(m_modem, &Modem::received, this, &ModemWorker::received);
            return true;
        }
    }
    return false;
}

bool ModemWorker::shutdown()
{
    if (m_modem && m_modem->getState() != Modem::STATE_PREINIT) {
        m_modem->shutdown();
        return true;
    }
    return false;
}

void ModemWorker::inputBlock(const QVector<double>& data)
{
    m_mutex.lock();
    if (m_modem) {
        while (!m_isBufferEmpty)
            m_bufferEmpty.wait(&m_mutex);

        // create an internal copy of the buffer
        m_buffer = data;
        m_isBufferEmpty = false;

        // signal that data is available
        m_bufferFull.wakeAll();
    }
    m_mutex.unlock();
}

void ModemWorker::terminate()
{
    m_mutex.lock();
    if (!m_terminate) {
        m_terminate = true;
        m_isBufferEmpty = false;
        m_bufferFull.wakeAll();
    }
    m_mutex.unlock();
}

void ModemWorker::run()
{
    /*m_mutex.lock();
    while (!m_terminate) {
        while (m_isBufferEmpty)
            m_bufferFull.wait(&m_mutex);

        if (!m_terminate) {
            m_modem->rxProcess(m_buffer);

            m_isBufferEmpty = true;
            m_bufferEmpty.wakeAll();
        }

        // allow the slots to be called
        qApp->processEvents();
    }
    m_mutex.unlock();

    emit finished();*/

    forever {

    }
}

void ModemWorker::send(QString /*message*/)
{
    // TODO: sending also has to be done within the run() function in order
    // to be processed inside the thread. Maybe signal state changes with conditions?

    //m_modem->txProcess(/*message*/);
}

Modem* ModemWorker::getModem()
{
    return m_modem;
}
