# Lichuan Servo Tuning / Конфигуратор сервоприводов Lichuan

[English](#english) · [Русский](#русский)

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
- UI in **RU / EN**, dark theme included

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
./build/servo_tuning            # --lang ru|en
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
- Интерфейс **RU / EN**, встроенная тёмная тема

**Сборка**
```bash
cmake -B build
cmake --build build -j
./build/servo_tuning            # --lang ru|en
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
