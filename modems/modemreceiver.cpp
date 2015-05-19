#include "modemreceiver.h"
#include "modem.h"

using namespace Digital::Internal;

ModemReceiver::ModemReceiver(QObject* parent, Modem* modem)
    : AudioConsumer(parent, 512),
      m_modem(modem)
{
}

void ModemReceiver::processAudio(const QVector<double>& data)
{
    m_modem->receive(data);
}
