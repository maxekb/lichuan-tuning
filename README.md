<div align="center"><img src="assets/logo.png" width="128" alt="Servo_Tuning logo"></div>

# Lichuan Servo Tuning / Конфигуратор сервоприводов Lichuan

[English](#english) · [Русский](#русский) · [中文](#中文)

---

## English

GTK4 (gtkmm4) configurator and tuning tool for **Lichuan A5-E / LC10E** servo
drives over **EtherCAT** (IgH master).

**Features**
- 199 parameters (groups P01–P0D) mapped to SDO objects `0x2000+group`, with
  names, units, ranges, value descriptions and raw↔display scaling
- SDO read/write through two backends: IgH **ecrt** API or the **`ethercat` CLI**;
  automatic fallback
- PDO/DC fast channel (CiA402 objects, 500 µs DC cycle) for real-time control
- Live monitor: HAL pins or P0B-group SDOs, 200 ms polling, fault history, CSV logging
- Parameter table with search / group filter, edit dialogs with value semantics
- UI in **RU / EN / ZH**, dark theme included

**Requirements**
- `libgtkmm-4.0-dev`, `cairomm`, pthreads
- optional: IgH `libethercat` + kernel headers (`ecrt.h`) for the fast channel;
  without them all SDO access goes through the `ethercat` CLI
- bus access needs the loaded IgH master and permissions on `/dev/EtherCAT0`
  (usually root)

**Build & run**
```bash
cmake -B build
cmake --build build -j
./build/servo_tuning            # --lang ru|en|zh
```

**Safety notes**
- Writes marked `SHUTDOWN` take effect after servo OFF→ON; `POWER_CYCLE` — after a
  full power cycle.
- Motor-related groups P0D.00 / P0D.01 / P0D.05 always require confirmation.
- Verify ranges against your drive's firmware: the built-in table follows the
  LC10E (EtherCAT) manual; some object sizes differ between firmware revisions.

**Architecture**  
GUI pages → `SdoClient` (slow channel) / `EcrtDomain` (PDO) → drive.
All bus I/O runs on worker threads; the GTK main thread never blocks.

---

## Русский

GUI-конфигуратор и инструмент настройки сервоприводов **Lichuan A5-E / LC10E**
по **EtherCAT** (мастер IgH), GTK4/gtkmm4.

**Возможности**
- 199 параметров (группы P01–P0D) с картами SDO-объектов `0x2000+группа`,
  единицами, диапазонами, расшифровкой значений и пересчётом raw↔экран
- Два канала SDO: API IgH **ecrt** или утилита **`ethercat`** (автопереключение)
- Быстрый канал PDO/DC (объекты CiA402, цикл 500 мкс)
- Монитор: HAL-пины или SDO группы P0B, опрос 200 мс, журнал аварий, лог в CSV
- Таблица параметров с поиском и фильтром по группам, диалоги редактирования
  со справкой по значениям
- Интерфейс **RU / EN / ZH**, встроенная тёмная тема

**Сборка**
```bash
cmake -B build
cmake --build build -j
./build/servo_tuning            # --lang ru|en|zh
```
Нужны `libgtkmm-4.0-dev`, `cairomm`, pthreads; опционально IgH (`libethercat`,
заголовки ядра) — без него SDO работает через CLI. Для доступа к шине —
загруженный мастер и права на `/dev/EtherCAT0` (обычно root).

**Безопасность**
- Параметры с режимом `SHUTDOWN` применяются после SERVO OFF→ON, `POWER_CYCLE` —
  после полного снятия питания.
- P0D.00 / P0D.01 / P0D.05 (моторные) всегда требуют подтверждения в GUI.
- Таблица параметров составлена по мануалу LC10E (EtherCAT); проверяйте
  соответствие прошивке вашего привода — у отдельных объектов размер и тип
  отличаются между ревизиями.

---

## 中文

基于 GTK4 (gtkmm4) 的 **Lichuan A5-E / LC10E** 伺服驱动器 **EtherCAT**（IgH
主站）参数配置与调试工具。

**功能**
- 199 个参数（P01–P0D 组）映射到 SDO 对象 `0x2000+组号`，包含名称、单位、
  范围、枚举值说明以及 raw↔显示值换算
- SDO 读写两种后端：IgH **ecrt** API 或 **`ethercat`** 命令行工具，自动回退
- PDO/DC 快速通道（CiA402 对象，500 µs DC 周期），用于实时控制
- 实时监视：HAL 引脚或 P0B 组 SDO，200 ms 轮询，故障历史，CSV 记录
- 参数表支持搜索与按组过滤；编辑对话框带枚举值语义说明
- 界面 **俄 / 英 / 中**，内置深色主题

**环境要求**
- `libgtkmm-4.0-dev`、`cairomm`、pthreads
- 可选：IgH `libethercat` + 内核头文件（`ecrt.h`）——没有它们时全部 SDO
  访问走 `ethercat` 命令行
- 总线访问需要已加载的 IgH 主站和 `/dev/EtherCAT0` 权限（通常需要 root）

**构建与运行**
```bash
cmake -B build
cmake --build build -j
./build/servo_tuning            # --lang ru|en|zh
```

**安全提示**
- 标记为 `SHUTDOWN` 的参数在 SERVO OFF→ON 后生效；`POWER_CYCLE` 需要完全
  重新上电。
- 电机相关参数 P0D.00 / P0D.01 / P0D.05 始终需要确认。
- 参数表依据 LC10E（EtherCAT）手册整理；请对照您驱动器的固件版本核实 —
  个别对象的字节大小和类型在不同固件版本之间存在差异。
