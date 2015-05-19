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

#include "audiodevicelist.h"

using namespace Digital::Internal;

AudioDeviceList::AudioDeviceList(QObject* parent)
    : QObject(parent)
{
}

AudioDeviceList::~AudioDeviceList()
{
    clear();
}

void AudioDeviceList::clear()
{
    foreach(AudioDeviceIn* dev, m_inDevices) {
        dev->stop();
        delete dev;
    }
    m_inDevices.clear();

    foreach(AudioDeviceOut* dev, m_outDevices) {
        dev->stop();
        delete dev;
    }
    m_outDevices.clear();
}

void AudioDeviceList::enumerate()
{
    // TODO: don't recreate instances, but add and remove

    clear();

    // get available input devices
    QList<QAudioDeviceInfo> inDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach(QAudioDeviceInfo dev, inDevices) {
        AudioDeviceIn* inputDevice = new AudioDeviceIn(this, dev);
        m_inDevices.push_back(inputDevice);
    }

    // get available output devices
    QList<QAudioDeviceInfo> outDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    foreach(QAudioDeviceInfo dev, outDevices) {
        AudioDeviceOut* outputDevice = new AudioDeviceOut(this, dev);
        m_outDevices.push_back(outputDevice);
    }
}

const QList<AudioDeviceIn*>& AudioDeviceList::getInputDevices() const
{
    return m_inDevices;
}

const QList<AudioDeviceOut*>& AudioDeviceList::getOutputDevices() const
{
    return m_outDevices;
}
