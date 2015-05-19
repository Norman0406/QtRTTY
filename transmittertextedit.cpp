#include "transmittertextedit.h"
#include <QFontDatabase>
#include <QKeyEvent>
#include <QDebug>

using namespace Digital::Internal;

TransmitterTextEdit::TransmitterTextEdit(QWidget* parent)
    : QTextEdit(parent),
      m_modem(0)
{
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    connect(this, &TransmitterTextEdit::textChanged, this, &TransmitterTextEdit::processText);
}

TransmitterTextEdit::TransmitterTextEdit(const QString& text, QWidget* parent)
    : QTextEdit(text, parent)
{
}

TransmitterTextEdit::~TransmitterTextEdit()
{
    unregisterModem();
}

void TransmitterTextEdit::registerModem(Digital::Internal::Modem* modem)
{
    m_modem = modem;
    connect(m_modem, &Modem::requestNextCharacter, this, &TransmitterTextEdit::prepareNextCharacter);
    connect(m_modem, &Modem::sent, this, &TransmitterTextEdit::characterSent);
    //connect(m_modem, &Modem::stateChanged, this, &TransmitterTextEdit::modemStateChanged);

    // TEMP
    //m_textPrepared = "Hallo";
    //updateText();
}

void TransmitterTextEdit::unregisterModem()
{
    if (m_modem) {
        disconnect(m_modem, &Modem::requestNextCharacter, this, &TransmitterTextEdit::prepareNextCharacter);
        disconnect(m_modem, &Modem::sent, this, &TransmitterTextEdit::characterSent);
        //disconnect(m_modem, &Modem::stateChanged, this, &TransmitterTextEdit::modemStateChanged);
        m_modem = 0;
    }
}

/*void TransmitterTextEdit::modemStateChanged(Digital::Internal::Modem::State state)
{
    Q_UNUSED(state);

    if (!m_modem->isTransmitting()) {
        clear();
    }
}*/

void TransmitterTextEdit::clear()
{
    m_textPrepared.clear();
    m_textInProgress.clear();
    m_textSent.clear();
    updateText();
}

void TransmitterTextEdit::processText()
{
    QString text = toPlainText();

    QString fixedText = m_textSent + m_textInProgress;
    QString remaining = text.mid(fixedText.length(), text.length() - fixedText.length());

    QString combined = fixedText + m_textPrepared;

    if (text == combined)
        return;

    m_textPrepared = remaining;
    updateText();
}

void TransmitterTextEdit::prepareNextCharacter()
{
    if (m_textPrepared.length() > 0) {
        char nextChar = m_textPrepared[0].toLatin1();
        if (m_modem->setNextCharacter(nextChar)) {
            m_textPrepared = m_textPrepared.mid(1, m_textPrepared.length() - 1);
            m_textInProgress += QChar(nextChar);

            updateText();
        }
    }
}

void TransmitterTextEdit::TransmitterTextEdit::characterSent(QChar character)
{
    if (character.isPrint() || character == '\n') {
        m_textSent += character;
        m_textInProgress.remove(0, 1);

        updateText();
    }
}

void TransmitterTextEdit::updateText()
{
    QTextCursor currentCursor = textCursor();

    QColor sentColor(Qt::red);
    QColor progressColor(Qt::darkGreen);
    QColor unsentColor(Qt::black);

    QString html;
    if (m_textSent.length() > 0) {
        html += "<s><font color=%1>";

        foreach (QChar character, m_textSent.toHtmlEscaped()) {
            if (character == '\n')
                html += "</font></s><br><s><font color=%1>";
            else if (character.isPrint())
                html += character;
            else
                qWarning() << "unregocnized character: " << character;
        }

        html += "</font></s>";
        html = html.arg(sentColor.name());
    }

    if (m_textInProgress.length() > 0) {
        html += "<font color=%1>";

        foreach (QChar character, m_textInProgress.toHtmlEscaped()) {
            if (character == '\n')
                html += "</font><br><font color=%1>";
            else if (character.isPrint())
                html += character;
            else
                qWarning() << "unregocnized character: " << character;
        }

        html += "</font>";
        html = html.arg(progressColor.name());
    }

    if (m_textPrepared.length() > 0) {
        html += "<font color=%1>";

        foreach (QChar character, m_textPrepared.toHtmlEscaped()) {
            if (character == '\n')
                html += "</font><br><font color=%1>";
            else if (character.isPrint())
                html += character;
            else
                qWarning() << "unregocnized character: " << character;
        }

        html += "</font>";
        html = html.arg(unsentColor.name());
    }

    setHtml(html);
    setTextCursor(currentCursor);
}
