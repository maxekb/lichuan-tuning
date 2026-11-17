#pragma once
#include <gtkmm.h>
#include "param_db.h"
#include "sdo_client.h"
#include <functional>
#include <thread>
#include <atomic>

class ParamEditDialog : public Gtk::Window {
public:
    ParamEditDialog(
        Gtk::Window& parent,
        const ParamDef& def,
        const Glib::ustring& current_val,
        SdoClient& sdo_client,
        std::function<void(const Glib::ustring&)> on_save
    );

    ~ParamEditDialog() override;

private:
    void build_ui();
    void adjust_value(int factor);
    void update_value_display();
    void on_save_clicked();
    void on_save_done();
    void on_cancel_clicked();

    const ParamDef& def_;
    SdoClient& sdo_client_;
    std::function<void(const Glib::ustring&)> on_save_;

    Gtk::Box main_vbox_;
    Gtk::Label title_label_;
    Gtk::Box button_row_;
    Gtk::Button btn_dec10_;
    Gtk::Button btn_dec1_;
    Gtk::Label value_label_;
    Gtk::Label unit_label_;
    Gtk::Button btn_inc1_;
    Gtk::Button btn_inc10_;
    Gtk::Label range_label_;
    Gtk::Label desc_label_;
    Gtk::Label error_label_;   // write errors go here; desc_label_ stays intact
    Gtk::Box action_row_;
    Gtk::Button save_btn_;
    Gtk::Button cancel_btn_;

    double current_val_;

    // SDO write is a blocking subprocess: it runs in save_thread_, result comes back via save_done_
    Glib::Dispatcher save_done_;
    std::thread save_thread_;
    std::atomic<bool> save_ok_ {false};
    std::atomic<uint32_t> save_abort_ {0};
    bool saving_ = false;
    // Value formatted at do_save time — what was actually sent to the device.
    // Reported back on completion so a mutated current_val_ can't leak into cached values.
    Glib::ustring saved_display_;
};
