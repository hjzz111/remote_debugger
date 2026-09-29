# Remote Debugger

Remote Debugger 是一个基于 STM32F103C8T6 的开源远程调试器项目，用于通过 USB CDC 串口控制目标设备上电、断电和复位，并支持应用固件 OTA 更新；pcb上预留UART、SPI、I2C接口，可模拟多种协议，具体功能将在后续更新。

项目包含调试器硬件工程、STM32 应用程序、Bootloader 和 Qt 桌面上位机。上位机按照“发送一帧、等待设备应答、再发送下一帧”的方式执行控制命令和 OTA 传输。

## 功能

- USB CDC 虚拟串口通信
- 连接确认与通信超时检测
- 控制目标设备上电和断电
- 模拟实体复位按键：按下时拉低，松开时释放（需将EXTRST引脚接到具有上拉电阻的复位引脚上）
- APP1/APP2 双区域 OTA
- OTA 固件版本和状态管理
- 512 字节固件分片传输
- Qt Widgets 图形化上位机
- 初次烧录元数据生成脚本

## 项目结构

```text
remote_debugger/
├─ bootloader/                  STM32 Bootloader 工程
├─ remote_debugger_stm32/       STM32 应用程序和 OTA 接收端
├─ remote_debugger_qt/          Qt 桌面上位机及构建目录
├─ remote_debugger_pcb/         PCB/EDA 工程
├─ generage_infofile.py         初次烧录元数据生成脚本
├─ infodata.bin                 已生成的元数据示例，使用前应重新生成
└─ README.md
```

## 硬件和开发环境

### 硬件

- MCU：STM32F103C8T6
- 通信接口：USB Full Speed CDC
- 调试接口：SWD

当前固件使用的主要控制引脚：

| 引脚 | 功能 | 有效状态 |
|---|---|---|
| `PB13` | 目标设备电源控制 | 高电平上电，低电平断电 |
| `PB14` | 目标设备通信控制 | 低电平开启，高电平关闭 |
| `PA1` | 目标设备复位控制 | 低电平复位，高电平释放 |
| `PA11` | USB DM | USB CDC |
| `PA12` | USB DP | USB CDC |

### 软件

- Keil MDK-ARM：编译 STM32 应用和 Bootloader
- STM32CubeMX：查看或更新 `.ioc` 配置
- Qt 5.14.2 MinGW 64-bit：编译桌面上位机
- Python 3：生成初次烧录元数据

## Flash 布局

当前固件按 STM32F103C8T6 的 64KB Flash 设计：

| 地址范围 | 大小 | 用途 |
|---|---:|---|
| `0x08000000–0x08000FFF` | 4KB | Bootloader |
| `0x08001000–0x080013FF` | 1KB | APP1/APP2 元数据 |
| `0x08001400–0x080087FF` | `0x7400` | APP1 运行区域 |
| `0x08008800–0x08008BFF` | 1KB | 保留 |
| `0x08008C00–0x0800FFFF` | `0x7400` | APP2 OTA 暂存区域 |

应用程序必须链接到 `0x08001400`。OTA 文件也必须由链接地址为 `0x08001400` 的工程生成；上位机将其暂存在 APP2，Bootloader 复位后再复制到 APP1。

元数据位于 `0x08001000`：

| 地址 | 类型 | 含义 |
|---|---|---|
| `0x08001000` | `uint32_t` | APP1 版本 |
| `0x08001004` | `uint32_t` | APP1 状态 |
| `0x08001008` | `uint32_t` | APP1 大小 |
| `0x0800100C` | `uint32_t` | APP2 版本 |
| `0x08001010` | `uint32_t` | APP2 状态 |
| `0x08001014` | `uint32_t` | APP2 大小 |

固件状态：

```text
0x00  IOS_BROKEN
0x01  IOS_READY
0x02  IOS_UPDATING
0x03  IOS_COPYING
```

## 编译 STM32 程序

### 应用程序

1. 使用 Keil 打开 `remote_debugger_stm32/MDK-ARM/remote_debugger_stm32.uvprojx`。
2. 确认 IROM1 起始地址为 `0x08001400`，大小为 `0x7400`。
3. 编译工程。
4. 工程的 After Build 命令会使用 `fromelf` 生成 `.bin` 文件。

生成的 OTA bin 通常位于：

```text
remote_debugger_stm32/MDK-ARM/remote_debugger_stm32/remote_debugger_stm32.bin
```

### Bootloader

1. 使用 Keil 打开 `bootloader/MDK-ARM/bootloader.uvprojx`。
2. Bootloader 的链接起始地址必须为 `0x08000000`。
3. 建议将 Bootloader IROM1 大小限制为 `0x1000`，避免代码增长后覆盖元数据页。
4. 编译并生成 HEX 或 BIN 文件。

## 初次烧录

首次生产烧录必须写入 Bootloader、APP1 和 APP1 元数据。**不要把应用程序链接到 `0x08000000` 作为最终镜像** 。

### 1. 生成元数据

在项目根目录运行：

```powershell
python .\generage_infofile.py `
  .\remote_debugger_stm32\MDK-ARM\remote_debugger_stm32\remote_debugger_stm32.bin `
  .\infodata.bin `
  --version 1
```

脚本自动读取 APP bin 的实际大小，生成一个 1KB 元数据页面。每次应用固件大小或版本变化后，都应重新生成该文件。

### 2. 烧录地址

| 文件 | 烧录地址 |
|---|---|
| Bootloader BIN | `0x08000000` |
| `infodata.bin` | `0x08001000` |
| APP BIN | `0x08001400` |

建议先整片擦除一次，再依次写入三个区域；写入过程中不要再次执行整片擦除。也可以将三个镜像合并成一个用于生产的 HEX 文件后一次烧录。

烧录完成并复位后，Bootloader 读取 APP1 元数据；当 APP1 状态为 `IOS_READY` 时跳转到 `0x08001400` 执行应用程序。

## 编译 Qt 上位机

Qt 工程文件：

```text
remote_debugger_qt/remote_debugger_qt/remote_debugger_qt.pro
```

依赖模块：

```qmake
QT += core gui widgets serialport
```

使用 Qt Creator：

1. 打开 `.pro` 文件。
2. 选择 Qt 5.14.2 MinGW 64-bit Kit。
3. 将构建配置切换为 `Release`。
4. 重新构建项目。
5. 使用 `windeployqt` 收集 Qt 和 MinGW 运行库。

示例：

```bat
"E:\Qt\5.14.2\mingw73_64\bin\windeployqt.exe" --release --compiler-runtime "path\to\release\remote_debugger_qt.exe"
```

发布时需要分发整个 Release 目录，至少应包含 EXE、Qt DLL、MinGW 运行库和 `platforms/qwindows.dll`，不能只复制 EXE。

## 使用上位机

1. 使用 USB 将调试器连接到计算机。
2. 启动上位机，点击“刷新”并选择对应的 CDC 串口。
3. 点击“连接”。只有该操作会发送连接确认命令。
4. 收到设备的 `0x00` 应答后，界面状态区域显示连接成功；连接失败或超时才会弹窗。
5. 点击“上电”或“断电”控制目标设备电源。
6. 按住“复位”按钮拉低复位信号，松开按钮后释放复位信号。

执行 OTA：

1. 选择链接地址为 `0x08001400` 的 APP `.bin` 文件。
2. 设置与当前 APP1 不同的固件版本号。
3. 点击“开始 OTA”。
4. 上位机以 512 字节分片发送，每收到一次成功应答后才发送下一帧。
5. OTA 完成后复位或重新上电；Bootloader 将 APP2 复制到 APP1并运行。

当前上位机限制 OTA 文件最大为 16484 字节。设备应答超时时间为 1.5 秒。

## 通信协议

USB CDC 串口配置为 `115200 8N1`。对于 USB CDC，波特率主要作为主机侧串口参数保留。

### 外层帧

```text
55 AA | command | payload_length_be16 | payload | checksum_be16
```

`checksum` 是 payload 每个字节的 16 位累加和，不是多项式 CRC。

命令字：

| 命令 | 名称 | payload |
|---:|---|---|
| `0x00` | 连接确认 | 空 |
| `0x01` | OTA | OTA 子帧 |
| `0x02` | 电源控制 | `00` 断电，`01` 上电 |
| `0x03` | 复位控制 | `00` 拉低，`01` 释放 |

OTA payload：

```text
START: 01 | seq | version_be32 | image_size_be16
DATA : 02 | seq | firmware_data
END  : 03 | seq
```

设备单字节应答：

```text
00  成功
FF  失败
```

## 当前限制

- OTA 分包序号尚未用于严格的连续性校验。
- 尚未实现整包固件 CRC/哈希或数字签名校验。
- Flash 擦除和写入错误处理仍需加强。
- Bootloader 尚未实现完整的镜像合法性检查和自动回滚。
- OTA 超时后不会自动重发数据帧，需要重新开始更新。
- 当前 Qt 上位机将 OTA 文件大小限制为 16484 字节。

该项目适合学习、原型验证和二次开发。用于无人值守、远程生产升级或安全敏感设备前，建议补充掉电保护、镜像认证、回滚、传输重试和更完整的故障恢复机制。

## 贡献

欢迎提交 Issue 和 Pull Request。提交修改时建议同时说明：

- 使用的硬件版本和 MCU 型号
- 编译工具及版本
- 问题复现步骤
- 串口日志或协议帧
- 是否修改了 Flash 布局或通信协议

## 许可证

项目原创代码、PCB 工程和文档采用 [MIT License](LICENSE) 开源。你可以自由使用、复制、修改、合并、发布、分发、再授权或销售本项目，但必须在副本或主要部分中保留原版权声明和 MIT 许可声明。

STM32 HAL、CMSIS、USB Device Library、Qt 以及其他第三方组件继续遵循其各自许可证。根目录 MIT License 只适用于本项目拥有许可权的原创部分，不替代第三方文件中已有的许可证和版权声明。
