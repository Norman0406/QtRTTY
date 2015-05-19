#ifndef MODEMTRANSMITTER_H
#define MODEMTRANSMITTER_H

#include "../audio/audioproducer.h"

#include <QMutex>
#include <QWaitCondition>
#include <QFuture>

namespace Digital {
namespace Internal {

class Modem;

class ModemTransmitter
        : public AudioProducer
{
    Q_OBJECT

public:
    ModemTransmitter(QObject*, qint32);
    ~ModemTransmitter();

    void writeValue(double value);
};

} // Internal
} // Digital

#endif // MODEMTRANSMITTER_H
