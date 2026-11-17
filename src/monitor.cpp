#include "monitor.h"
#include "sdo_client.h"
#include <thread>
#include <cstdio>
#include <array>
#include <sstream>
#include <chrono>
#include <cmath>

Monitor::Monitor(uint16_t slave_pos)
    : slave_pos_(slave_pos) {}

bool Monitor::start(int interval_ms)
{
    interval_ms_ = interval_ms;
    running_ = true;
    if (loop_thread_.joinable())
        loop_thread_.join();
    loop_thread_ = std::thread([this]() { loop(); });
    return true;
}

void Monitor::stop()
{
    running_ = false;
    if (loop_thread_.joinable())
        loop_thread_.join();
}

bool Monitor::start_csv_logging(const std::string& path)
{
    csv_file_.open(path);
    if (!csv_file_.is_open()) return false;
    csv_file_ << "timestamp_ms,speed_rpm,torque_pct,position_mm,"
              << "current_a,bus_v,temp_c,error_mm\n";
    csv_logging_ = true;
    return true;
}

void Monitor::stop_csv_logging()
{
    csv_logging_ = false;
    if (csv_file_.is_open())
        csv_file_.close();
}

MonitorData Monitor::read_once()
{
    MonitorData data;
    data.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    read_hal_pins(data);
    read_sdo_p0b(data);
    return data;
}

void Monitor::loop()
{
    while (running_) {
        auto t1 = std::chrono::steady_clock::now();

        MonitorData data = read_once();

        if (callback_)
            callback_(data);

        if (csv_logging_ && csv_file_.is_open()) {
            csv_file_ << data.timestamp_ms << ","
                      << data.speed_rpm << ","
                      << data.torque_pct << ","
                      << data.position_mm << ","
                      << data.current_a << ","
                      << data.bus_volts << ","
                      << data.temp_c << ","
                      << data.error_mm << "\n";
        }

        auto elapsed = std::chrono::steady_clock::now() - t1;
        auto sleep_ms = interval_ms_
            - std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        if (sleep_ms > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
}

void Monitor::read_hal_pins(MonitorData& data)
{
    std::array<char, 1024> buffer;
    FILE* pipe = popen("halcmd show pin pid-butterfly.* 2>/dev/null", "r");
    if (!pipe) return;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::string line(buffer.data());
        auto last_space = line.rfind(' ');
        if (last_space == std::string::npos) continue;
        std::string val_str = line.substr(last_space + 1);

        if (line.find("f-error") != std::string::npos) {
            data.error_mm = std::abs(std::stod(val_str));
        } else if (line.find("vel-fb") != std::string::npos || line.find("vel-fb-actual") != std::string::npos) {
            data.speed_rpm = std::abs(std::stod(val_str)) / 2097152.0 * 4.0;
        } else if (line.find("pos-fb") != std::string::npos && line.find("abs") == std::string::npos) {
            data.position_mm = std::stod(val_str) / 2097152.0;
        }
    }
    pclose(pipe);
}

void Monitor::read_sdo_p0b(MonitorData& data)
{
    if (!sdo_client_) return;

    auto r = sdo_client_->read(0x200B, 0x03, SdoType::INT16);
    if (r.success && r.data.size() >= 2) {
        int16_t raw = r.data[0] | (r.data[1] << 8);
        data.torque_pct = raw * 0.1;
    }

    r = sdo_client_->read(0x200B, 0x19, SdoType::UINT16);
    if (r.success && r.data.size() >= 2) {
        uint16_t raw = r.data[0] | (r.data[1] << 8);
        data.current_a = raw * 0.01;
    }

    r = sdo_client_->read(0x200B, 0x1B, SdoType::UINT16);
    if (r.success && r.data.size() >= 2) {
        uint16_t raw = r.data[0] | (r.data[1] << 8);
        data.bus_volts = raw * 0.1;
    }

    r = sdo_client_->read(0x200B, 0x1C, SdoType::UINT16);
    if (r.success && r.data.size() >= 2) {
        uint16_t raw = r.data[0] | (r.data[1] << 8);
        data.temp_c = raw;
    }

    r = sdo_client_->read(0x200B, 0x23, SdoType::UINT16);
    if (r.success && r.data.size() >= 2) {
        data.fault_code = r.data[0] | (r.data[1] << 8);
        data.status = (data.fault_code == 0) ? "OK" : "FAULT:" + std::to_string(data.fault_code);
    }
}
