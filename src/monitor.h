#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <fstream>
#include <thread>

class SdoClient;

struct MonitorData {
    double speed_rpm = 0.0;
    double torque_pct = 0.0;
    double position_mm = 0.0;
    double error_mm = 0.0;
    double current_a = 0.0;
    double bus_volts = 0.0;
    double temp_c = 25.0;
    uint16_t fault_code = 0;
    std::string status = "OK";
    uint64_t timestamp_ms = 0;
};

class Monitor {
public:
    using DataCallback = std::function<void(const MonitorData&)>;

    Monitor(uint16_t slave_pos = 0);
    ~Monitor() { stop(); }

    bool start(int interval_ms = 200);
    void stop();
    bool is_running() const { return running_; }

    void set_callback(DataCallback cb) { callback_ = cb; }

    // Inject a persistent SdoClient reference (avoids creating one per poll cycle)
    void set_sdo_client(SdoClient* sdo) { sdo_client_ = sdo; }

    bool start_csv_logging(const std::string& path);
    void stop_csv_logging();
    bool is_csv_logging() const { return csv_logging_; }

    MonitorData read_once();

private:
    void loop();
    void read_hal_pins(MonitorData& data);
    void read_sdo_p0b(MonitorData& data);

    uint16_t slave_pos_;
    bool running_ = false;
    int interval_ms_ = 200;
    DataCallback callback_;
    bool csv_logging_ = false;
    std::ofstream csv_file_;
    std::thread loop_thread_;
    SdoClient* sdo_client_ = nullptr; // injected, not owned
};
