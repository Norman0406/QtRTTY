#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <QTextEdit>
#include "audio/audiodevicelist.h"
#include "audio/audiodeviceout.h"
#include "audio/audiodevicein.h"
#include "modems/modemreceiver.h"
#include "modems/modemtransmitter.h"
#include "modems/modemrtty.h"

namespace Ui {
class MainWindow;
}

using namespace Digital::Internal;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbStartInput_clicked();
    void on_pbStopInput_clicked();
    void on_cbInputDevices_currentIndexChanged(int index);
    void on_pbStartOutput_clicked();
    void on_pbStopOutput_clicked();
    void on_cbOutputDevices_currentIndexChanged(int index);
    void characterReceived(char);
    void characterSent(char);

private:
    void appendCharacter(QTextEdit*, char, QColor);

    Ui::MainWindow *ui;

    AudioDeviceList     m_deviceList;
    AudioDeviceIn*      m_inDevice;
    AudioDeviceOut*     m_outDevice;
    ModemRTTY*          m_modem;
};

#endif // MAINWINDOW_H
