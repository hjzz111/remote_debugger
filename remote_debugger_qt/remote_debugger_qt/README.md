# Remote Debugger Qt 上位机

该程序使用 Qt Widgets 和 Qt Serial Port，实现设备电源控制、目标复位和 STM32 OTA 下载。

## 使用方法

1. 连接设备后点击“刷新”，选择对应的 USB CDC 串口。
2. 点击“连接”。只有这个按钮会发送连接确认命令；收到 `0x00` 后状态栏显示已连接，超时或失败才弹出错误窗口。
3. 上电和断电命令在对应按钮按下时发送。
4. 复位按钮按下时拉低复位信号，保持按住，松开时释放复位信号。
5. OTA 时选择链接地址为 `0x08001400` 的 `.bin` 文件。
6. 设置一个与设备当前 APP1 不同的固件版本号。
7. 点击“开始 OTA”。程序以 512 字节分片发送，每收到一次 `0x00` 应答后才发送下一帧。
8. 进度达到 100% 后，复位或重新上电 STM32，Bootloader 会把 APP2 复制到 APP1并运行。

设备返回 `0xFF` 或 1.5 秒内没有返回应答时，本次操作会终止。数据帧超时不会自动重发，因为当前固件尚未利用帧序号识别重复包。

## 协议

外层帧：

```text
55 AA | command | payload_length_be16 | payload | checksum_be16
```

`checksum` 是 payload 所有字节的 16 位累加和。

OTA payload：

```text
START: 01 | seq | version_be32 | image_size_be16
DATA : 02 | seq | firmware_data
END  : 03 | seq
```

命令字：

```text
00 连接确认
01 OTA
02 电源控制，payload 00=断电、01=上电
03 复位控制，payload 00=拉低、01=释放
```

## 构建

项目依赖 Qt Widgets 和 Qt Serial Port：

```text
QT += core gui widgets serialport
```

已使用 Qt 5.14.2 MinGW 64-bit 验证编译通过。

## 许可证

本上位机原创代码作为 Remote Debugger 项目的一部分，采用 [MIT License](../../LICENSE) 开源。Qt 框架及其运行库继续遵循 Qt 自身的许可证。
