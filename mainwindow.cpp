#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_deviceList(this),
    m_outDevice(0),
    m_inDevice(0)
{
    ui->setupUi(this);

    m_modem = new ModemRTTY(this);
    //connect(m_modem, &Modem::received, this, &MainWindow::characterReceived);
    //connect(m_modem, &Modem::sent, this, &MainWindow::characterSent);

    //connect(ui->txtSend, SIGNAL(sendCharacter(QChar)), m_modem, SLOT(transmit(QChar)));
    //connect(ui->txtSend, &TransmitterTextEdit::removeCharacter, m_modem, &Modem::removeCharacter);
    //connect(m_modem, &Modem::sent, ui->txtSend, &TransmitterTextEdit::characterSent);

    ui->txtSend->registerModem(m_modem);

    m_deviceList.enumerate();

    const QList<AudioDeviceIn*> inDevices = m_deviceList.getInputDevices();
    if (inDevices.size() > 0) {
        ui->cbInputDevices->clear();
        ui->cbInputDevices->setEnabled(true);
    }

    foreach (AudioDeviceIn* device, inDevices) {
        ui->cbInputDevices->addItem(device->getDeviceName());
    }

    const QList<AudioDeviceOut*> outDevices = m_deviceList.getOutputDevices();
    if (outDevices.size() > 0) {
        ui->cbOutputDevices->clear();
        ui->cbOutputDevices->setEnabled(true);
    }

    foreach (AudioDeviceOut* device, outDevices) {
        ui->cbOutputDevices->addItem(device->getDeviceName());
    }
}

MainWindow::~MainWindow()
{
    delete m_modem;
}

void MainWindow::on_pbStartInput_clicked()
{
    if (m_modem && m_modem->startRx()) {
        ui->pbStartInput->setEnabled(false);
        ui->pbStopInput->setEnabled(true);
        ui->txtRcvd->setEnabled(true);
    }
}

void MainWindow::on_pbStopInput_clicked()
{
    if (m_modem && m_modem->stopRx()) {
        ui->pbStartInput->setEnabled(true);
        ui->pbStopInput->setEnabled(false);
        ui->txtRcvd->setEnabled(false);
    }
}

void MainWindow::on_cbInputDevices_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    if (m_inDevice) {
        m_inDevice->close();
    }

    m_inDevice = m_deviceList.getInputDevices()[index];
    if (m_outDevice)
        m_inDevice->setFormat(m_outDevice->getFormat());
    else {
        m_inDevice->setSampleRate(8000);
        m_inDevice->setChannelCount(1);
        m_inDevice->setSampleSize(16);
    }
    m_inDevice->init();

    m_modem->init(m_inDevice, m_outDevice);

    qDebug() << "initialized input device: " << m_inDevice->getDeviceName();

    ui->pbStartInput->setEnabled(true);
    ui->pbStopInput->setEnabled(false);
    ui->txtRcvd->setEnabled(false);
}

void MainWindow::on_pbStartOutput_clicked()
{
    if (m_modem && m_modem->startTx()) {
        ui->pbStartOutput->setEnabled(false);
        ui->pbStopOutput->setEnabled(true);
        ui->txtSend->setEnabled(true);
    }
}

void MainWindow::on_pbStopOutput_clicked()
{
    if (m_modem && m_modem->stopTx()) {
        ui->pbStartOutput->setEnabled(true);
        ui->pbStopOutput->setEnabled(false);
        ui->txtSend->setEnabled(false);
    }
}

void MainWindow::on_cbOutputDevices_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    if (m_outDevice) {
        m_outDevice->close();
    }

    m_outDevice = m_deviceList.getOutputDevices()[index];
    if (m_inDevice)
        m_outDevice->setFormat(m_inDevice->getFormat());
    else {
        m_outDevice->setSampleRate(8000);
        m_outDevice->setChannelCount(1);
    }
    m_outDevice->init();

    m_modem->init(m_inDevice, m_outDevice);

    qDebug() << "initialized output device: " << m_outDevice->getDeviceName();

    ui->pbStartOutput->setEnabled(true);
    ui->pbStopOutput->setEnabled(false);
    ui->txtSend->setEnabled(false);
}

void MainWindow::characterReceived(char character)
{
    appendCharacter(ui->txtRcvd, character, Qt::black);
}

void MainWindow::characterSent(char character)
{
    appendCharacter(ui->txtRcvd, character, Qt::red);
}

void MainWindow::appendCharacter(QTextEdit* textEdit, char character, QColor color)
{
    // store previous positions of cursor and scroll bars
    bool horSliderDown = textEdit->horizontalScrollBar()->isSliderDown();
    int horScroll = textEdit->horizontalScrollBar()->value();
    bool verSliderDown = textEdit->verticalScrollBar()->isSliderDown();
    int verScroll = textEdit->verticalScrollBar()->value();
    QTextCursor prevCursor = textEdit->textCursor();

    // only scroll to the bottom if the current scroll bar position has already been at the bottom
    /*bool verScrollMax = false;
    int verMax = textEdit->verticalScrollBar()->maximum();
    if (verScroll == verMax)
        verScrollMax = true;

    bool horScrollMax = false;
    if (horScroll == textEdit->horizontalScrollBar()->maximum())
        horScrollMax = true;*/

    bool scrollMax = false;

    textEdit->moveCursor(QTextCursor::End);

    QString htmlChar(character);
    if (character == ' ')
        htmlChar = QString("&nbsp;");
    else if (character == '\n')
        htmlChar = QString("<br>");
    else
        htmlChar = QString("<font color=%1>%2</font>").arg(color.name()).arg(QString(character).toHtmlEscaped());

    textEdit->textCursor().insertHtml(htmlChar);

    // restore positions
    textEdit->setTextCursor(prevCursor);

    // scroll to right
    /*if (horScrollMax && verScrollMax)
        horScroll = 0;
    else if (horScrollMax)
        horScroll = textEdit->horizontalScrollBar()->maximum();*/
    textEdit->horizontalScrollBar()->setValue(horScroll);

    // scroll to bottom
    /*if (verScrollMax)
        verScroll = textEdit->verticalScrollBar()->maximum();*/
    textEdit->verticalScrollBar()->setValue(verScroll);
}
