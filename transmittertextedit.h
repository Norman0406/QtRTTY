#ifndef TRANSMITTERTEXTBOX_H
#define TRANSMITTERTEXTBOX_H

#include <QTextEdit>
#include "modems/modem.h"

class TransmitterTextEdit
        : public QTextEdit
{
    Q_OBJECT

public:
    explicit TransmitterTextEdit(QWidget* parent = 0);
    explicit TransmitterTextEdit(const QString& text, QWidget* parent = 0);
    virtual ~TransmitterTextEdit();

    void registerModem(Digital::Internal::Modem*);
    void unregisterModem();
    void clear();

public slots:
    void characterSent(QChar);
    void prepareNextCharacter();
    void txChanged(bool);
    void modemInitialized(bool);

private:
    void processText();
    void updateText();

    QString m_textPrepared;     // text that has been entered but can still be changed and hasn't yet been sent to the modem
    QString m_textInProgress;   // text that is currently in progress of being sent
    QString m_textSent;         // text that the modem has actually sent
    Digital::Internal::Modem* m_modem;
};

#endif // TRANSMITTERTEXTBOX_H
