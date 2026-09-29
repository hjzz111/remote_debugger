// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Remote Debugger contributors

#include "widget.h"
#include "ui_widget.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QTimer>

namespace {

constexpr quint8 CommandConnect = 0x00U;
constexpr quint8 CommandOta = 0x01U;
constexpr quint8 CommandPower = 0x02U;
constexpr quint8 CommandReset = 0x03U;

constexpr quint8 OtaMessageStart = 0x01U;
constexpr quint8 OtaMessageData = 0x02U;
constexpr quint8 OtaMessageEnd = 0x03U;

void appendUInt16Be(QByteArray &data, quint16 value)
{
    data.append(static_cast<char>((value >> 8U) & 0xFFU));
    data.append(static_cast<char>(value & 0xFFU));
}

void appendUInt32Be(QByteArray &data, quint32 value)
{
    data.append(static_cast<char>((value >> 24U) & 0xFFU));
    data.append(static_cast<char>((value >> 16U) & 0xFFU));
    data.append(static_cast<char>((value >> 8U) & 0xFFU));
    data.append(static_cast<char>(value & 0xFFU));
}

} // namespace

constexpr int Widget::FirmwareMaxSize;
constexpr int Widget::OtaChunkSize;
constexpr int Widget::AckTimeoutMs;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_serial(new QSerialPort(this))
    , m_ackTimer(new QTimer(this))
    , m_pendingAction(PendingAction::None)
    , m_otaOffset(0)
    , m_pendingChunkSize(0)
    , m_otaSequence(1U)
    , m_otaVersion(1U)
    , m_portVerified(false)
    , m_operationBusy(false)
    , m_resetPressed(false)
    , m_resetReleaseQueued(false)
{
    ui->setupUi(this);

    m_ackTimer->setSingleShot(true);
    m_ackTimer->setInterval(AckTimeoutMs);

    connect(ui->refreshButton, &QPushButton::clicked,
            this, &Widget::refreshSerialPorts);
    connect(ui->connectButton, &QPushButton::clicked,
            this, &Widget::connectDevice);
    connect(ui->browseButton, &QPushButton::clicked,
            this, &Widget::browseFirmware);
    connect(ui->otaButton, &QPushButton::clicked,
            this, &Widget::startOta);
    connect(ui->powerOnButton, &QPushButton::pressed,
            this, &Widget::powerOn);
    connect(ui->powerOffButton, &QPushButton::pressed,
            this, &Widget::powerOff);
    connect(ui->resetButton, &QPushButton::pressed,
            this, &Widget::resetPressed);
    connect(ui->resetButton, &QPushButton::released,
            this, &Widget::resetReleased);
    connect(m_serial, &QSerialPort::readyRead,
            this, &Widget::serialReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred,
            this, &Widget::serialError);
    connect(m_ackTimer, &QTimer::timeout,
            this, &Widget::ackTimeout);

    connect(ui->portComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int) {
                if (!m_operationBusy && m_serial->isOpen() &&
                    m_serial->portName() != ui->portComboBox->currentData().toString()) {
                    m_serial->close();
                    m_portVerified = false;
                    ui->statusLabel->setText(tr("串口已切换，下一次操作时重新连接"));
                }
            });

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    refreshSerialPorts();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::refreshSerialPorts()
{
    const QString previousPort = ui->portComboBox->currentData().toString();

    ui->portComboBox->blockSignals(true);
    ui->portComboBox->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        QString displayName = port.portName();
        if (!port.description().isEmpty()) {
            displayName += QStringLiteral(" - ") + port.description();
        }
        ui->portComboBox->addItem(displayName, port.portName());
    }

    const int previousIndex = ui->portComboBox->findData(previousPort);
    if (previousIndex >= 0) {
        ui->portComboBox->setCurrentIndex(previousIndex);
    }

    ui->portComboBox->blockSignals(false);

    if (ports.isEmpty()) {
        ui->statusLabel->setText(tr("未发现串口"));
    } else if (!m_serial->isOpen()) {
        ui->statusLabel->setText(tr("请选择设备串口"));
    }
}

void Widget::browseFirmware()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("选择固件"),
        ui->pathEdit->text(),
        tr("二进制固件 (*.bin);;所有文件 (*.*)"));

    if (!path.isEmpty()) {
        ui->pathEdit->setText(path);
        const QFileInfo info(path);
        ui->statusLabel->setText(
            tr("已选择 %1，%2 字节").arg(info.fileName()).arg(info.size()));
    }
}

bool Widget::ensureSerialOpen()
{
    const QString portName = ui->portComboBox->currentData().toString();
    if (portName.isEmpty()) {
        QMessageBox::warning(this, tr("串口"), tr("没有可用串口，请连接设备后刷新。"));
        return false;
    }

    if (m_serial->isOpen() && m_serial->portName() == portName) {
        return true;
    }

    if (m_serial->isOpen()) {
        m_serial->close();
    }

    m_serial->setPortName(portName);
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(
            this,
            tr("打开串口失败"),
            tr("无法打开 %1：%2").arg(portName, m_serial->errorString()));
        return false;
    }

    m_serial->clear(QSerialPort::AllDirections);
    m_portVerified = false;
    ui->statusLabel->setText(tr("已打开 %1，正在确认设备").arg(portName));
    return true;
}

void Widget::connectDevice()
{
    if (m_operationBusy) {
        return;
    }

    if (!ensureSerialOpen()) {
        return;
    }

    m_portVerified = false;
    setOperationBusy(true);
    ui->statusLabel->setText(tr("正在等待设备连接确认"));

    if (!sendFrame(CommandConnect, QByteArray(), PendingAction::Handshake)) {
        abortOperation(tr("连接确认帧发送失败"));
    }
}

bool Widget::beginOperation()
{
    if (m_operationBusy) {
        return false;
    }

    if (!m_serial->isOpen() || !m_portVerified) {
        QMessageBox::warning(this, tr("设备未连接"), tr("请先选择串口并点击“连接”。"));
        return false;
    }

    setOperationBusy(true);
    return true;
}

void Widget::setOperationBusy(bool busy)
{
    m_operationBusy = busy;
    ui->portComboBox->setEnabled(!busy);
    ui->refreshButton->setEnabled(!busy);
    ui->connectButton->setEnabled(!busy);
    ui->pathEdit->setEnabled(!busy);
    ui->browseButton->setEnabled(!busy);
    ui->versionSpinBox->setEnabled(!busy);
    ui->otaButton->setEnabled(!busy);
    ui->powerOnButton->setEnabled(!busy);
    ui->powerOffButton->setEnabled(!busy);
    ui->resetButton->setEnabled(!busy || m_resetPressed);
}

void Widget::finishOperation(const QString &message)
{
    m_ackTimer->stop();
    m_pendingAction = PendingAction::None;
    m_resetPressed = false;
    m_resetReleaseQueued = false;
    setOperationBusy(false);
    ui->statusLabel->setText(message);
}

void Widget::abortOperation(const QString &message)
{
    m_ackTimer->stop();
    m_pendingAction = PendingAction::None;
    m_firmware.clear();
    m_pendingChunkSize = 0;
    m_resetPressed = false;
    m_resetReleaseQueued = false;
    setOperationBusy(false);
    ui->statusLabel->setText(message);
    QMessageBox::critical(this, tr("操作失败"), message);
}

QByteArray Widget::buildFrame(quint8 command, const QByteArray &payload) const
{
    QByteArray frame;
    frame.reserve(payload.size() + 7);
    frame.append(static_cast<char>(0x55));
    frame.append(static_cast<char>(0xAA));
    frame.append(static_cast<char>(command));
    appendUInt16Be(frame, static_cast<quint16>(payload.size()));
    frame.append(payload);

    quint16 checksum = 0U;
    for (char byte : payload) {
        checksum = static_cast<quint16>(
            checksum + static_cast<unsigned char>(byte));
    }
    appendUInt16Be(frame, checksum);
    return frame;
}

bool Widget::sendFrame(quint8 command,
                       const QByteArray &payload,
                       PendingAction action)
{
    if (!m_serial->isOpen() || m_pendingAction != PendingAction::None) {
        return false;
    }

    const QByteArray frame = buildFrame(command, payload);
    const qint64 queuedBytes = m_serial->write(frame);
    if (queuedBytes != frame.size()) {
        return false;
    }

    m_serial->flush();
    m_pendingAction = action;
    m_ackTimer->start();
    return true;
}

void Widget::serialReadyRead()
{
    const QByteArray received = m_serial->readAll();

    if (m_pendingAction == PendingAction::None) {
        return;
    }

    for (char byte : received) {
        const quint8 value = static_cast<quint8>(byte);
        if (value == 0x00U || value == 0xFFU) {
            handleAck(value == 0x00U);
            break;
        }
    }
}

void Widget::handleAck(bool success)
{
    const PendingAction completedAction = m_pendingAction;
    m_ackTimer->stop();
    m_pendingAction = PendingAction::None;

    if (!success) {
        if (completedAction == PendingAction::Handshake) {
            m_portVerified = false;
        }
        abortOperation(tr("设备返回失败应答 0xFF"));
        return;
    }

    switch (completedAction) {
    case PendingAction::Handshake:
        m_portVerified = true;
        finishOperation(tr("已连接：%1").arg(m_serial->portName()));
        break;
    case PendingAction::PowerOn:
        finishOperation(tr("上电命令执行成功"));
        break;
    case PendingAction::PowerOff:
        finishOperation(tr("断电命令执行成功"));
        break;
    case PendingAction::ResetAssert:
        if (m_resetReleaseQueued || !m_resetPressed) {
            sendResetCommand(false);
        } else {
            ui->statusLabel->setText(tr("复位保持中，松开按钮后释放"));
        }
        break;
    case PendingAction::ResetRelease:
        finishOperation(tr("复位完成"));
        break;
    case PendingAction::OtaStart:
        ui->statusLabel->setText(tr("设备已进入OTA模式"));
        sendNextOtaData();
        break;
    case PendingAction::OtaData:
        m_otaOffset += m_pendingChunkSize;
        m_pendingChunkSize = 0;
        ++m_otaSequence;
        ui->progressBar->setValue(
            static_cast<int>((m_otaOffset * 100) / m_firmware.size()));
        if (m_otaOffset < m_firmware.size()) {
            sendNextOtaData();
        } else {
            sendOtaEnd();
        }
        break;
    case PendingAction::OtaEnd:
        ui->progressBar->setValue(100);
        m_firmware.clear();
        finishOperation(tr("固件已写入APP2；请复位或重新上电设备以完成安装"));
        QMessageBox::information(
            this,
            tr("OTA完成"),
            tr("固件已完整写入APP2。\n请复位或重新上电设备，Bootloader将把固件复制到APP1并运行。"));
        break;
    case PendingAction::None:
        break;
    }
}

void Widget::ackTimeout()
{
    const bool wasHandshake = m_pendingAction == PendingAction::Handshake;
    m_pendingAction = PendingAction::None;
    if (wasHandshake) {
        m_portVerified = false;
    }

    abortOperation(tr("等待设备应答超时。OTA数据帧不会自动重发，请重新开始操作。"));
}

void Widget::serialError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::ResourceError) {
        return;
    }

    const QString message = tr("串口连接断开：%1").arg(m_serial->errorString());
    m_portVerified = false;
    if (m_serial->isOpen()) {
        m_serial->close();
    }

    if (m_operationBusy) {
        abortOperation(message);
    } else {
        ui->statusLabel->setText(message);
    }
}

void Widget::powerOn()
{
    if (beginOperation()) {
        sendPowerCommand(true);
    }
}

void Widget::powerOff()
{
    if (beginOperation()) {
        sendPowerCommand(false);
    }
}

void Widget::sendPowerCommand(bool on)
{
    QByteArray payload;
    payload.append(static_cast<char>(on ? 0x01 : 0x00));
    ui->statusLabel->setText(on ? tr("正在执行上电") : tr("正在执行断电"));

    if (!sendFrame(CommandPower,
                   payload,
                   on ? PendingAction::PowerOn : PendingAction::PowerOff)) {
        abortOperation(tr("电源控制命令发送失败"));
    }
}

void Widget::resetPressed()
{
    if (m_operationBusy) {
        return;
    }

    if (!m_serial->isOpen() || !m_portVerified) {
        QMessageBox::warning(this, tr("设备未连接"), tr("请先选择串口并点击“连接”。"));
        return;
    }

    m_resetPressed = true;
    m_resetReleaseQueued = false;
    setOperationBusy(true);
    ui->statusLabel->setText(tr("正在拉低复位信号"));
    sendResetCommand(true);
}

void Widget::resetReleased()
{
    if (!m_resetPressed) {
        return;
    }

    m_resetPressed = false;
    setOperationBusy(true);

    if (m_pendingAction == PendingAction::ResetAssert) {
        m_resetReleaseQueued = true;
        ui->statusLabel->setText(tr("等待复位按下应答后释放"));
        return;
    }

    if (m_operationBusy && m_pendingAction == PendingAction::None) {
        sendResetCommand(false);
    }
}

void Widget::sendResetCommand(bool asserted)
{
    QByteArray payload;
    payload.append(static_cast<char>(asserted ? 0x00 : 0x01));

    if (!sendFrame(CommandReset,
                   payload,
                   asserted ? PendingAction::ResetAssert
                            : PendingAction::ResetRelease)) {
        abortOperation(tr("复位控制命令发送失败"));
    }
}

void Widget::startOta()
{
    QFile firmwareFile(ui->pathEdit->text());
    if (!firmwareFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("固件"), tr("无法打开所选固件文件。"));
        return;
    }

    const QByteArray firmware = firmwareFile.readAll();
    if (firmware.isEmpty()) {
        QMessageBox::warning(this, tr("固件"), tr("固件文件为空。"));
        return;
    }
    if (firmware.size() > FirmwareMaxSize) {
        QMessageBox::warning(
            this,
            tr("固件过大"),
            tr("当前固件大小为 %1 字节，设备允许的最大大小为 %2 字节。")
                .arg(firmware.size())
                .arg(FirmwareMaxSize));
        return;
    }

    m_firmware = firmware;
    m_otaVersion = static_cast<quint32>(ui->versionSpinBox->value());
    m_otaOffset = 0;
    m_pendingChunkSize = 0;
    m_otaSequence = 1U;
    ui->progressBar->setValue(0);

    if (beginOperation()) {
        sendOtaStart();
    } else {
        m_firmware.clear();
    }
}

void Widget::sendOtaStart()
{
    QByteArray payload;
    payload.reserve(8);
    payload.append(static_cast<char>(OtaMessageStart));
    payload.append(static_cast<char>(0x00));
    appendUInt32Be(payload, m_otaVersion);
    appendUInt16Be(payload, static_cast<quint16>(m_firmware.size()));

    ui->statusLabel->setText(tr("正在初始化OTA并擦除APP2"));
    if (!sendFrame(CommandOta, payload, PendingAction::OtaStart)) {
        abortOperation(tr("OTA开始帧发送失败"));
    }
}

void Widget::sendNextOtaData()
{
    const int remaining = static_cast<int>(m_firmware.size() - m_otaOffset);
    const int chunkSize = qMin(OtaChunkSize, remaining);
    const QByteArray chunk = m_firmware.mid(m_otaOffset, chunkSize);

    QByteArray payload;
    payload.reserve(chunk.size() + 2);
    payload.append(static_cast<char>(OtaMessageData));
    payload.append(static_cast<char>(m_otaSequence));
    payload.append(chunk);

    m_pendingChunkSize = chunkSize;
    ui->statusLabel->setText(
        tr("正在发送固件：%1 / %2 字节")
            .arg(m_otaOffset + chunkSize)
            .arg(m_firmware.size()));

    if (!sendFrame(CommandOta, payload, PendingAction::OtaData)) {
        abortOperation(tr("OTA数据帧发送失败"));
    }
}

void Widget::sendOtaEnd()
{
    QByteArray payload;
    payload.reserve(2);
    payload.append(static_cast<char>(OtaMessageEnd));
    payload.append(static_cast<char>(m_otaSequence));

    ui->statusLabel->setText(tr("正在完成OTA"));
    if (!sendFrame(CommandOta, payload, PendingAction::OtaEnd)) {
        abortOperation(tr("OTA结束帧发送失败"));
    }
}

