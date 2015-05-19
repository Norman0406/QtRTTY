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

#ifndef MODEM_H
#define MODEM_H

#include <QObject>
#include <QString>
#include <QVector>
#include <cmath>

#include <QAudioFormat>
#include <QWaitCondition>
#include <QQueue>
#include <QThread>
#include <QMutex>

namespace Digital {
namespace Internal {

#define TWO_PI 2.0 * M_PI

class ModemReceiver;
class ModemTransmitter;
class AudioDeviceIn;
class AudioDeviceOut;

class Modem
        : public QObject
{
    Q_OBJECT

public:
    enum Capability
    {
        CAP_RX = 1 << 0,    // can receive
        CAP_TX = 1 << 1,    // can transmit
        CAP_AFC = 1 << 2,   // has automatic frequency control
        CAP_REV = 1 << 3,   // has reverse mode (USB - LSB)
        CAP_MULT = 1 << 4,  // ability to scan multiple channels in parallel (channel browser)
        CAP_SCOPE = 1 << 5, // defines a scope widget
    };

    /*enum State {
        STATE_NONE, //
        STATE_RX,
        STATE_TX,
    };*/

    enum AFCSpeed
    {
        AFC_SLOW,
        AFC_NORMAL,
        AFC_FAST
    };

    virtual ~Modem();

    bool init(AudioDeviceIn*, AudioDeviceOut*);
    void restart();
    void shutdown();

    virtual QString getType() const = 0;

    void            setFrequency(double);
    void            setAFCSpeed(AFCSpeed);
    bool            setAFC(bool);
    bool            setReverse(bool);
    bool            isTransmitting() const;
    bool            isReceiving() const;

    bool            hasCapability(Capability) const;
    double          getFrequency() const;
    virtual double  getBandwidth() const = 0;
    double          getMetric() const;
    AFCSpeed        getAFCSpeed() const;
    bool            getAFC() const;
    bool            isReverse() const;
    double          isSquelchOpen() const; // metric > squelch ? true : false

public slots:
    void process();

    // transmitting
    bool startTx();
    bool startTxAuto(); // automatically stops transmitting after no more characters are received
    bool setNextCharacter(char);
    bool clearNextCharacter();
    bool stopTx();

    // receiving
    bool startRx();
    void receive(const QVector<double>&);
    bool stopRx();

signals:
    /// \brief Emitted after a character has been sent
    void sent(char);

    /// \brief Emitted when a character is prepared for sending (i.e. it is now being processed and
    /// will be sent to the soundcard soon). At this time, a new character can be set via setNextCharacter().
    void requestNextCharacter();

    void txChanged(bool txOn);
    void rxChanged(bool rxOn);
    void initialized(bool);

    void received(char);

    void frequencyChanged(double);
    void bandwidthChanged(double);

protected:
    Modem(unsigned, QObject*);

    enum InternalState
    {
        INTSTATE_PREINIT,      // modem has not yet been initialized, init() needs to be called
        INTSTATE_READY,        // the thread has started running, but the modem is neither receiving nor transmitting
        INTSTATE_RX,           // the modem is currently in receiving mode
        INTSTATE_TX_STARTING,  // the modem started transmitting
        INTSTATE_TX,           // the modem is in transmitting mode
        INTSTATE_TX_STOPPING,  // the modem is stopping the transmission
        INTSTATE_SHUTDOWN      // the modem thread is shutting down, next state is STATE_PREINIT
    };

    virtual bool    iInit() = 0;
    virtual void    iRestart() = 0;
    virtual void    iShutdown() = 0;
    virtual void    iRxProcess(const QVector<double>&) = 0;
    virtual void    iTxProcess() = 0;
    virtual double  computeMetric() const = 0;
    bool            getNextChar(QChar&);
    bool            writeSample(double);
    InternalState   getInternalState() const;

    double          getFrqErr() const;
    void            adjustFrequency(double);   // AFC
    int             getSampleRate() const;

private:
    void setInternalState(InternalState);

    QThread*        m_baseThread;
    QMutex          m_waitMutex;
    QWaitCondition  m_waitForData;
    QThread*        m_thread;
    QMutex          m_extMutex;
    QWaitCondition  m_extWaitCond;

    QWaitCondition  m_txStoppedCond;

    QMutex          m_stateChangedMutex;
    QWaitCondition  m_stateChangedCond;

    QVector<double> m_inputBlock;
    bool            m_hasInputData;

    AudioDeviceIn*      m_deviceIn;
    AudioDeviceOut*     m_deviceOut;
    ModemReceiver*      m_receiver;
    ModemTransmitter*   m_transmitter;

    QMutex              m_nextCharacterMutex;
    bool                m_hasNextCharacter;
    int                 m_nextCharacter;
    bool                m_autoMode;

    QAudioFormat    m_format;
    unsigned        m_capability;
    double          m_metric;
    double          m_frequency;
    AFCSpeed        m_afcSpeed;
    double          m_freqErr;
    bool            m_afc;
    bool            m_reverse;
    InternalState   m_internalState;
    InternalState   m_requestedState;
};

} // namespace Internal
} // namespace Digital

#endif // MODEM_H
