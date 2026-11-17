#include "gui_monitor_page.h"
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "i18n.h"

MonitorPage::MonitorPage(SdoClient& sdo) : sdo_client_(sdo)
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_spacing(6);
    set_margin(8);
    build_ui();

    data_dispatcher_.connect(sigc::mem_fun(*this, &MonitorPage::on_data_updated));
}

MonitorPage::~MonitorPage() {
    stop_monitoring();
    faults_running_ = false;
    if (faults_thread_.joinable())
        faults_thread_.join();
}

void MonitorPage::build_ui()
{
    // Control bar
    control_bar_.set_orientation(Gtk::Orientation::HORIZONTAL);
    control_bar_.set_spacing(6);
    control_bar_.set_margin_bottom(6);

    start_btn_.set_label(tr("monitor_start"));
    start_btn_.set_css_classes({"btn-success"});
    start_btn_.signal_toggled().connect([this] {
        if (start_btn_.get_active()) start_monitoring();
        else stop_monitoring();
    });

    stop_btn_.set_label(tr("monitor_stop"));
    stop_btn_.set_css_classes({"btn-danger"});
    stop_btn_.signal_clicked().connect([this] { stop_monitoring(); });

    csv_btn_.set_label(tr("monitor_csv"));
    csv_btn_.signal_toggled().connect([this] {
        csv_btn_.set_label(csv_btn_.get_active() ? tr("monitor_csv_on") : tr("monitor_csv"));
    });

    interval_spin_.set_range(50, 5000);
    interval_spin_.set_value(200);
    interval_spin_.set_increments(50, 200);

    control_bar_.append(start_btn_);
    control_bar_.append(stop_btn_);
    control_bar_.append(csv_btn_);
    interval_label_ = Gtk::make_managed<Gtk::Label>(tr("monitor_interval"));
    control_bar_.append(*interval_label_);
    control_bar_.append(interval_spin_);
    append(control_bar_);

    // DRO grid
    auto make_dro = [&](const Glib::ustring& label,
                         Glib::RefPtr<Gtk::Label>& val_label,
                         const Glib::ustring& key,
                         Glib::RefPtr<Gtk::Label>& dro_label) -> Gtk::Box& {
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        box->set_spacing(2);
        box->set_margin(4);
        box->set_css_classes({"dro-frame"});

        val_label = Glib::RefPtr<Gtk::Label>(Gtk::make_managed<Gtk::Label>("---"));
        val_label->set_css_classes({"dro-value"});

        dro_label = Glib::RefPtr<Gtk::Label>(Gtk::make_managed<Gtk::Label>(label));
        dro_label->set_css_classes({"dro-label"});

        box->append(*val_label);
        box->append(*dro_label);
        return *box;
    };

    dro_grid_.set_row_spacing(4);
    dro_grid_.set_column_spacing(4);

    dro_grid_.attach(make_dro(tr("monitor_speed"), speed_label_, speed_dro_key_, speed_dro_label_), 0, 0, 1, 1);
    dro_grid_.attach(make_dro(tr("monitor_torque"), torque_label_, torque_dro_key_, torque_dro_label_), 1, 0, 1, 1);
    dro_grid_.attach(make_dro(tr("monitor_position"), position_label_, position_dro_key_, position_dro_label_), 2, 0, 1, 1);
    dro_grid_.attach(make_dro(tr("monitor_error"), error_label_, error_dro_key_, error_dro_label_), 3, 0, 1, 1);
    dro_grid_.attach(make_dro(tr("monitor_current"), current_label_, current_dro_key_, current_dro_label_), 0, 1, 1, 1);
    dro_grid_.attach(make_dro(tr("monitor_busv"), bus_v_label_, busv_dro_key_, busv_dro_label_), 1, 1, 1, 1);
    dro_grid_.attach(make_dro(tr("monitor_temp"), temp_label_, temp_dro_key_, temp_dro_label_), 2, 1, 1, 1);

    auto status_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    status_box->set_spacing(2);
    status_box->set_margin(4);
    status_box->set_css_classes({"dro-frame"});
    status_label_ = Glib::RefPtr<Gtk::Label>(Gtk::make_managed<Gtk::Label>(tr("monitor_status")));
    status_label_->set_css_classes({"dro-status"});
    auto st_lbl = Gtk::make_managed<Gtk::Label>(tr("monitor_status"));
    st_lbl->set_css_classes({"dro-label"});
    status_box->append(*status_label_);
    status_box->append(*st_lbl);
    dro_grid_.attach(*status_box, 3, 1, 1, 1);
    status_box_ptr_ = status_box;

    append(dro_grid_);

    // Graph
    graph_box_.set_orientation(Gtk::Orientation::VERTICAL);
    graph_box_.set_margin_top(8);
    graph_box_.set_vexpand(true);

    speed_graph_.add_series(tr("tuning_speed"), Gdk::RGBA("#4fc3f7"), 0, 60);
    torque_graph_.add_series(tr("monitor_torque"), Gdk::RGBA("#ff7043"), 0, 300);
    graph_box_.append(speed_graph_);
    graph_box_.append(torque_graph_);

    graph_vs_time_label_ = Gtk::make_managed<Gtk::Label>(tr("tuning_speed_vs_time"));
    graph_box_.append(*graph_vs_time_label_);
    append(graph_box_);

    // Stats
    stats_label_.set_text(tr("tuning_stats"));
    stats_label_.set_margin_top(4);
    stats_label_.set_css_classes({"stats-label"});
    append(stats_label_);

    // Fault History
    fault_panel_.set_orientation(Gtk::Orientation::VERTICAL);
    fault_panel_.set_spacing(4);
    fault_panel_.set_margin_top(12);

    fault_header_label_ = Gtk::make_managed<Gtk::Label>(tr("fault_history"));
    fault_header_label_->set_css_classes({"section-header"});
    fault_panel_.append(*fault_header_label_);

    read_faults_btn_.set_label(tr("btn_read_faults"));
    read_faults_btn_.signal_clicked().connect(sigc::mem_fun(*this, &MonitorPage::read_faults));
    fault_panel_.append(read_faults_btn_);

    fault_store_ = Gio::ListStore<FaultRow>::create();
    fault_sel_ = Gtk::SingleSelection::create(fault_store_);
    fault_view_.set_model(fault_sel_);
    fault_view_.add_css_class("fault-list");

    auto make_fault_col = [&](const Glib::ustring& title, int flex,
        std::function<Glib::ustring(const Glib::RefPtr<FaultRow>&)> getter) {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem>& item) {
            item->set_child(*Gtk::make_managed<Gtk::Label>("", Gtk::Align::START));
        });
        factory->signal_bind().connect([getter](const Glib::RefPtr<Gtk::ListItem>& item) {
            auto label = dynamic_cast<Gtk::Label*>(item->get_child());
            if (!label) return;
            auto row = std::dynamic_pointer_cast<FaultRow>(item->get_item());
            if (row) label->set_text(getter(row));
        });
        auto col = Gtk::ColumnViewColumn::create(title, factory);
        col->set_resizable(true);
        if (flex > 0) col->set_expand(true);
        return col;
    };

    auto col_slot = make_fault_col(tr("fault_slot"), 0, [](auto r) { return r->slot; });
    fault_view_.append_column(col_slot);
    fault_columns_.push_back(col_slot);

    auto col_code = make_fault_col(tr("fault_code"), 0, [](auto r) { return r->code; });
    fault_view_.append_column(col_code);
    fault_columns_.push_back(col_code);

    auto col_time = make_fault_col(tr("fault_time"), 1, [](auto r) { return r->timestamp; });
    fault_view_.append_column(col_time);
    fault_columns_.push_back(col_time);

    auto col_speed = make_fault_col(tr("fault_speed"), 0, [](auto r) { return r->speed; });
    fault_view_.append_column(col_speed);
    fault_columns_.push_back(col_speed);

    auto col_u = make_fault_col(tr("fault_u_a"), 0, [](auto r) { return r->u_cur; });
    fault_view_.append_column(col_u);
    fault_columns_.push_back(col_u);

    auto col_v = make_fault_col(tr("fault_v_a"), 0, [](auto r) { return r->v_cur; });
    fault_view_.append_column(col_v);
    fault_columns_.push_back(col_v);

    auto col_bus = make_fault_col(tr("fault_bus_v"), 0, [](auto r) { return r->bus_v; });
    fault_view_.append_column(col_bus);
    fault_columns_.push_back(col_bus);

    auto fscroll = Gtk::make_managed<Gtk::ScrolledWindow>();
    fscroll->set_child(fault_view_);
    fscroll->set_size_request(-1, 180);
    fault_panel_.append(*fscroll);
    append(fault_panel_);


}

void MonitorPage::start_monitoring()
{
    if (monitoring_.load()) return;
    monitoring_.store(true);
    start_btn_.set_label(tr("tuning_running"));
    start_btn_.set_css_classes({"btn-running"});
    speed_graph_.clear();
    torque_graph_.clear();

    int interval_ms = static_cast<int>(interval_spin_.get_value());
    if (interval_ms < 50) interval_ms = 50;

    poll_thread_ = std::thread([this, interval_ms]() {
        auto next = std::chrono::steady_clock::now();
        while (monitoring_.load()) {
            read_all();
            data_dispatcher_.emit();
            next += std::chrono::milliseconds(interval_ms);
            std::this_thread::sleep_until(next);
        }
    });
}

void MonitorPage::stop_monitoring()
{
    monitoring_.store(false);
    if (poll_thread_.joinable())
        poll_thread_.join();
    start_btn_.set_active(false);
    start_btn_.set_label(tr("monitor_start"));
    start_btn_.set_css_classes({"btn-success"});
}

void MonitorPage::on_data_updated()
{
    std::lock_guard<std::mutex> lock(data_mutex_);

    auto fmt = [](double v, int p) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(p) << v;
        return ss.str();
    };

    if (!data_.comm_ok) {
        speed_label_->set_text("ERR");
        torque_label_->set_text("ERR");
        position_label_->set_text("ERR");
        error_label_->set_text("ERR");
        current_label_->set_text("ERR");
        bus_v_label_->set_text("ERR");
        temp_label_->set_text("ERR");
    } else {
        speed_label_->set_text(fmt(data_.speed, 0));
        torque_label_->set_text(fmt(data_.torque, 1) + " %");
        position_label_->set_text(fmt(data_.position, 0));
        error_label_->set_text(fmt(data_.error, 0));
        current_label_->set_text(fmt(data_.current, 2) + " A");
        bus_v_label_->set_text(fmt(data_.bus_voltage, 1) + " V");
        temp_label_->set_text(fmt(data_.temperature, 0) + " \xc2\xb0" + "C");
    }
    status_label_->set_text("DI:" + data_.di_status + " DO:" + data_.do_status);

    speed_graph_.append_data(0, data_.speed);
    torque_graph_.append_data(0, data_.torque);

    // Update fault table
    fault_store_->remove_all();
    for (auto& f : fault_data_) {
        std::ostringstream code;
        code << "0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << f.code;

        fault_store_->append(FaultRow::create(
            std::to_string(f.slot),
            code.str(),
            std::to_string(f.timestamp) + "s",
            std::to_string(f.speed) + " rpm",
            Glib::ustring::sprintf("%.2f", f.u_current / 100.0),
            Glib::ustring::sprintf("%.2f", f.v_current / 100.0),
            Glib::ustring::sprintf("%.1f", f.bus_voltage / 10.0)
        ));
    }
    read_faults_btn_.set_sensitive(true);
}

void MonitorPage::read_all()
{
    auto to_int16 = [](const SdoResult& r) -> int16_t {
        if (!r.success || r.data.size() < 2) return 0;
        return static_cast<int16_t>(r.data[0] | (r.data[1] << 8));
    };
    auto to_int32 = [](const SdoResult& r) -> int32_t {
        if (!r.success || r.data.size() < 4) return 0;
        return static_cast<int32_t>(r.data[0] | (r.data[1] << 8) | (r.data[2] << 16) | (r.data[3] << 24));
    };
    auto to_uint16 = [](const SdoResult& r) -> uint16_t {
        if (!r.success || r.data.size() < 2) return 0;
        return static_cast<uint16_t>(r.data[0] | (r.data[1] << 8));
    };
    auto to_hex = [](uint16_t v, int w) {
        std::ostringstream ss;
        ss << "0x" << std::hex << std::uppercase << std::setw(w) << std::setfill('0') << v;
        return ss.str();
    };

    auto spd = sdo_client_.read(0x200B, 0x01, SdoType::INT16);
    auto trq = sdo_client_.read(0x200B, 0x03, SdoType::INT16);
    auto pos = sdo_client_.read(0x200B, 0x08, SdoType::INT32);
    auto err = sdo_client_.read(0x200B, 0x10, SdoType::INT32);
    auto cur = sdo_client_.read(0x200B, 0x19, SdoType::UINT16);
    auto bus = sdo_client_.read(0x200B, 0x1B, SdoType::UINT16);
    auto tmp = sdo_client_.read(0x200B, 0x1C, SdoType::UINT16);
    auto di_in = sdo_client_.read(0x200B, 0x04, SdoType::INT16);
    auto di_out = sdo_client_.read(0x200B, 0x06, SdoType::INT16);

    bool any_fail = !spd.success || !trq.success || !pos.success || !err.success
                 || !cur.success || !bus.success || !tmp.success
                 || !di_in.success || !di_out.success;

    std::lock_guard<std::mutex> lock(data_mutex_);
    data_.comm_ok = !any_fail;
    data_.speed = to_int16(spd);
    data_.torque = to_int16(trq) * 0.1;
    data_.position = to_int32(pos);
    data_.error = to_int32(err);
    data_.current = to_uint16(cur) * 0.01;
    data_.bus_voltage = to_uint16(bus) * 0.1;
    data_.temperature = to_uint16(tmp);
    data_.di_status = to_hex(static_cast<uint16_t>(to_int16(di_in)), 4);
    data_.do_status = to_hex(static_cast<uint16_t>(to_int16(di_out)), 4);
}

void MonitorPage::read_faults()
{
    read_faults_btn_.set_sensitive(false);

    if (faults_thread_.joinable())
        faults_thread_.join();
    faults_running_ = true;
    faults_thread_ = std::thread([this]() {
        std::array<FaultRecord, 10> faults{};

        auto to_u16 = [](const SdoResult& r) -> uint16_t {
            if (!r.success || r.data.size() < 2) return 0;
            return static_cast<uint16_t>(r.data[0] | (r.data[1] << 8));
        };
        auto to_i16 = [](const SdoResult& r) -> int16_t {
            if (!r.success || r.data.size() < 2) return 0;
            return static_cast<int16_t>(r.data[0] | (r.data[1] << 8));
        };
        auto to_i32 = [](const SdoResult& r) -> int32_t {
            if (!r.success || r.data.size() < 4) return 0;
            return static_cast<int32_t>(r.data[0] | (r.data[1] << 8) | (r.data[2] << 16) | (r.data[3] << 24));
        };

        for (uint16_t slot = 0; slot < 10 && faults_running_.load(); ++slot) {
            std::vector<uint8_t> sel = {uint8_t(slot & 0xFF), uint8_t((slot >> 8) & 0xFF)};
            auto wr = sdo_client_.write(0x200B, 0x22, sel, SdoType::UINT16);

            auto code = sdo_client_.read(0x200B, 0x23, SdoType::UINT16);
            auto ts   = sdo_client_.read(0x200B, 0x24, SdoType::INT32);
            auto spd  = sdo_client_.read(0x200B, 0x26, SdoType::INT16);
            auto u_i  = sdo_client_.read(0x200B, 0x27, SdoType::INT16);
            auto v_i  = sdo_client_.read(0x200B, 0x28, SdoType::INT16);
            auto bus  = sdo_client_.read(0x200B, 0x29, SdoType::UINT16);

            faults[slot].slot = slot;
            faults[slot].code = to_u16(code);
            faults[slot].timestamp = to_i32(ts);
            faults[slot].speed = to_i16(spd);
            faults[slot].u_current = to_i16(u_i);
            faults[slot].v_current = to_i16(v_i);
            faults[slot].bus_voltage = to_u16(bus);
        }

        faults_running_ = false;

        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            fault_data_ = faults;
        }
        data_dispatcher_.emit();
    });
}

void MonitorPage::refresh()
{
    speed_graph_.set_series_name(0, tr("tuning_speed"));
    torque_graph_.set_series_name(0, tr("monitor_torque"));

    if (speed_dro_label_) speed_dro_label_->set_text(tr(speed_dro_key_));
    if (torque_dro_label_) torque_dro_label_->set_text(tr(torque_dro_key_));
    if (position_dro_label_) position_dro_label_->set_text(tr(position_dro_key_));
    if (error_dro_label_) error_dro_label_->set_text(tr(error_dro_key_));
    if (current_dro_label_) current_dro_label_->set_text(tr(current_dro_key_));
    if (busv_dro_label_) busv_dro_label_->set_text(tr(busv_dro_key_));
    if (temp_dro_label_) temp_dro_label_->set_text(tr(temp_dro_key_));
    if (status_dro_label_) status_dro_label_->set_text(tr(status_dro_key_));

    // Control bar buttons
    start_btn_.set_label(tr("monitor_start"));
    stop_btn_.set_label(tr("monitor_stop"));
    if (!csv_btn_.get_active())
        csv_btn_.set_label(tr("monitor_csv"));
    else
        csv_btn_.set_label(tr("monitor_csv_on"));

    if (interval_label_) interval_label_->set_text(tr("monitor_interval"));
    if (graph_vs_time_label_) graph_vs_time_label_->set_text(tr("tuning_speed_vs_time"));

    stats_label_.set_text(tr("tuning_stats"));

    // Fault section
    if (fault_header_label_) fault_header_label_->set_text(tr("fault_history"));
    read_faults_btn_.set_label(tr("btn_read_faults"));

    // Fault column titles
    if (fault_columns_.size() >= 7) {
        fault_columns_[0]->set_title(tr("fault_slot"));
        fault_columns_[1]->set_title(tr("fault_code"));
        fault_columns_[2]->set_title(tr("fault_time"));
        fault_columns_[3]->set_title(tr("fault_speed"));
        fault_columns_[4]->set_title(tr("fault_u_a"));
        fault_columns_[5]->set_title(tr("fault_v_a"));
        fault_columns_[6]->set_title(tr("fault_bus_v"));
    }

    if (status_box_ptr_) {
        auto st_lbl = status_box_ptr_->get_children()[1];
        if (st_lbl) {
            Gtk::Label* lbl = dynamic_cast<Gtk::Label*>(st_lbl);
            if (lbl) lbl->set_text(tr("monitor_status"));
        }
    }
}
