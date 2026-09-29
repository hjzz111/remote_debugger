// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Remote Debugger contributors

#ifndef WIDGET_H
#define WIDGET_H

#include <QByteArray>
#include <QSerialPort>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class QTimer;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void refreshSerialPorts();
    void connectDevice();
    void browseFirmware();
    void startOta();
    void powerOn();
    void powerOff();
    void resetPressed();
    void resetReleased();
    void serialReadyRead();
    void ackTimeout();
    void serialError(QSerialPort::SerialPortError error);

private:
    enum class PendingAction {
        None,
        Handshake,
        PowerOn,
        PowerOff,
        ResetAssert,
        ResetRelease,
        OtaStart,
        OtaData,
        OtaEnd
    };

    static constexpr int FirmwareMaxSize = 16484;
    static constexpr int OtaChunkSize = 512;
    static constexpr int AckTimeoutMs = 1500;

    bool ensureSerialOpen();
    bool beginOperation();
    void setOperationBusy(bool busy);
    void finishOperation(const QString &message);
    void abortOperation(const QString &message);

    QByteArray buildFrame(quint8 command, const QByteArray &payload) const;
    bool sendFrame(quint8 command,
                   const QByteArray &payload,
                   PendingAction action);
    void handleAck(bool success);

    void sendPowerCommand(bool on);
    void sendResetCommand(bool asserted);
    void sendOtaStart();
    void sendNextOtaData();
    void sendOtaEnd();

    Ui::Widget *ui;
    QSerialPort *m_serial;
    QTimer *m_ackTimer;
    PendingAction m_pendingAction;

    QByteArray m_firmware;
    qsizetype m_otaOffset;
    int m_pendingChunkSize;
    quint8 m_otaSequence;
    quint32 m_otaVersion;

    bool m_portVerified;
    bool m_operationBusy;
    bool m_resetPressed;
    bool m_resetReleaseQueued;
};
#endif // WIDGET_H
