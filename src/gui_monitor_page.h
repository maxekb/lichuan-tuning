#pragma once
#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/togglebutton.h>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <array>
#include "gui_graph.h"

struct FaultRow : public Glib::Object {
    Glib::ustring slot;
    Glib::ustring code;
    Glib::ustring timestamp;
    Glib::ustring speed;
    Glib::ustring u_cur;
    Glib::ustring v_cur;
    Glib::ustring bus_v;

    static Glib::RefPtr<FaultRow> create(
        const Glib::ustring& s, const Glib::ustring& c,
        const Glib::ustring& t, const Glib::ustring& sp,
        const Glib::ustring& u, const Glib::ustring& v,
        const Glib::ustring& b)
    {
        return Glib::make_refptr_for_instance<FaultRow>(
            new FaultRow(s, c, t, sp, u, v, b));
    }

private:
    FaultRow(const Glib::ustring& s, const Glib::ustring& c,
             const Glib::ustring& t, const Glib::ustring& sp,
             const Glib::ustring& u, const Glib::ustring& v,
             const Glib::ustring& b)
        : Glib::Object(), slot(s), code(c), timestamp(t),
          speed(sp), u_cur(u), v_cur(v), bus_v(b) {}
};
#include "sdo_client.h"

class MonitorPage : public Gtk::Box {
public:
    explicit MonitorPage(SdoClient& sdo);
    ~MonitorPage() override;

    void on_data_updated();
    void start_monitoring();
    void stop_monitoring();
    void refresh();

private:
    void build_ui();
    void build_dro_row(Gtk::Box& parent, const Glib::ustring& label,
                       Glib::RefPtr<Gtk::Label>& value_label, const Glib::ustring& unit);

    Glib::RefPtr<Gtk::Label> speed_dro_label_, torque_dro_label_, position_dro_label_,
        error_dro_label_, current_dro_label_, busv_dro_label_, temp_dro_label_;
    Glib::RefPtr<Gtk::Label> status_dro_label_;

    Gtk::Box* status_box_ptr_ = nullptr;

    Glib::ustring speed_dro_key_ = "monitor_speed", torque_dro_key_ = "monitor_torque",
        position_dro_key_ = "monitor_position", error_dro_key_ = "monitor_error",
        current_dro_key_ = "monitor_current", busv_dro_key_ = "monitor_busv",
        temp_dro_key_ = "monitor_temp", status_dro_key_ = "monitor_status";

    Gtk::Box control_bar_;
    Gtk::ToggleButton start_btn_;
    Gtk::Button stop_btn_;
    Gtk::ToggleButton csv_btn_;
    Gtk::SpinButton interval_spin_;

    Gtk::Grid dro_grid_;
    Glib::RefPtr<Gtk::Label> speed_label_;
    Glib::RefPtr<Gtk::Label> torque_label_;
    Glib::RefPtr<Gtk::Label> position_label_;
    Glib::RefPtr<Gtk::Label> error_label_;
    Glib::RefPtr<Gtk::Label> current_label_;
    Glib::RefPtr<Gtk::Label> bus_v_label_;
    Glib::RefPtr<Gtk::Label> temp_label_;
    Glib::RefPtr<Gtk::Label> status_label_;

    Gtk::Box graph_box_;
    GraphWidget speed_graph_;
    GraphWidget torque_graph_;

    Gtk::Label stats_label_;
    Gtk::Label* interval_label_ = nullptr;
    Gtk::Label* graph_vs_time_label_ = nullptr;
    Gtk::Label* fault_header_label_ = nullptr;

    std::atomic<bool> monitoring_{false};
    std::atomic<bool> faults_running_{false};

    SdoClient& sdo_client_;
    Glib::Dispatcher data_dispatcher_;
    std::thread poll_thread_;
    std::thread faults_thread_;
    std::mutex data_mutex_;

    struct LiveData {
        double speed = 0;
        double torque = 0;
        double position = 0;
        double error = 0;
        double current = 0;
        double bus_voltage = 0;
        double temperature = 0;
        std::string di_status;
        std::string do_status;
        bool comm_ok = true;
    };
    LiveData data_;

    void read_all();
    void read_faults();

    struct FaultRecord {
        uint16_t slot = 0;
        uint16_t code = 0;
        int32_t timestamp = 0;
        int16_t speed = 0;
        int16_t u_current = 0;
        int16_t v_current = 0;
        uint16_t bus_voltage = 0;
    };
    std::array<FaultRecord, 10> fault_data_;

    Gtk::Box fault_panel_;
    Gtk::Button read_faults_btn_;
    Gtk::ColumnView fault_view_;
    Glib::RefPtr<Gio::ListStore<FaultRow>> fault_store_;
    Glib::RefPtr<Gtk::SingleSelection> fault_sel_;
    std::vector<Glib::RefPtr<Gtk::ColumnViewColumn>> fault_columns_;
};
