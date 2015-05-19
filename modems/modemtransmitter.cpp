#include "modemtransmitter.h"
#include "modem.h"
#include <QtConcurrent/QtConcurrent>

using namespace Digital::Internal;

ModemTransmitter::ModemTransmitter(QObject* parent, qint32 bufferSize)
    : AudioProducer(parent, bufferSize)
{
}

ModemTransmitter::~ModemTransmitter()
{
}

void ModemTransmitter::writeValue(double value)
{
    write(value);
}
