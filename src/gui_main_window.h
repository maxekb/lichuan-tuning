#pragma once

#include <gtkmm.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/notebook.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/stack.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/popover.h>
#include "sdo_client.h"
#include "ecrt_domain.h"
#include <memory>
#include <thread>
#include <vector>
#include <utility>
#include <string>

class ParamPage;
class MonitorPage;

class MainWindow : public Gtk::ApplicationWindow {
public:
    explicit MainWindow(bool show_splash = true);
    ~MainWindow() override;

private:
    void build_ui();
    void build_menu();
    void create_splash_screen();
    void update_status_bar();
    void on_language_changed();
    void on_scan_devices();
    void apply_scan_results();
    void on_device_changed();
    Gtk::Box main_box_;
    Gtk::HeaderBar header_bar_;
    Gtk::Stack stack_;
    Gtk::StackSwitcher stack_switcher_;
    Gtk::Box status_box_;
    Gtk::Label status_label_;
    Gtk::Label device_label_;
    Gtk::ComboBoxText device_combo_;
    Gtk::Button scan_btn_;
    Gtk::ComboBoxText lang_combo_;
    Gtk::MenuButton menu_button_;
    Gtk::Popover popover_;
    Gtk::Box menu_box_;
    Gtk::Button btn_about_;
    Gtk::Button btn_quit_;

    // Own window controls: the Breeze GTK4 theme hides CSD titlebutton glyphs
    // under a dark headerbar and its icon pipeline is not overrideable via
    // CSS, so we draw text-glyph buttons (immune to theme icon quirks).
    Gtk::Button btn_win_min_;
    Gtk::Button btn_win_max_;
    Gtk::Button btn_win_close_;


    sigc::connection conn_lang_changed_;
    sigc::connection conn_device_changed_;

    SdoClient sdo_client_{0};
    EcrtDomain ecrt_domain_;
    std::unique_ptr<ParamPage> param_page_;
    std::unique_ptr<MonitorPage> monitor_page_;

    // Device scan is a blocking CLI subprocess: it runs in scan_thread_,
    // results come back via scan_done_ to the main thread (see CLAUDE.md)
    std::thread scan_thread_;
    Glib::Dispatcher scan_done_;
    std::vector<std::pair<std::string, std::string>> scan_results_; // id -> label
    std::string scan_preserve_id_;
    bool scanning_ = false;
};
