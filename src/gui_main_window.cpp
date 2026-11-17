#include "gui_main_window.h"
#include "gui_param_page.h"
#include "gui_monitor_page.h"
#include "i18n.h"

#include <gtkmm/dialog.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/cssprovider.h>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <memory>

MainWindow::~MainWindow() {
    // Join the scan worker before anything else dies: it must not emit
    // scan_done_ into a destroyed window.
    if (scan_thread_.joinable())
        scan_thread_.join();
#ifdef HAVE_ETHERCAT_H
    ecrt_domain_.close();
#endif
}

MainWindow::MainWindow(bool show_splash)
{
    // Restore saved language from settings.json before building UI
    {
        const char* home = getenv("HOME");
        std::string dir = std::string(home ? home : "/tmp") + "/.config/servo_tuning";
        std::ifstream sf(dir + "/settings.json");
        if (sf.is_open()) {
            std::string line;
            while (std::getline(sf, line)) {
                auto pos = line.find("\"lang\"");
                if (pos != std::string::npos) {
                    auto q1 = line.find('"', pos + 6);
                    auto q2 = line.find('"', q1 + 1);
                    if (q1 != std::string::npos && q2 != std::string::npos) {
                        std::string saved = line.substr(q1 + 1, q2 - q1 - 1);
                        if (!saved.empty()) setenv("LANGUAGE", saved.c_str(), 1);
                    }
                }
            }
        }
    }

    set_title(tr("window_title"));
    set_default_size(1024, 768);

    // SDO connect first (uses CLI backend if no master yet)
    sdo_client_.connect();

    scan_done_.connect([this] { apply_scan_results(); });

    build_menu();
    build_ui();
    on_scan_devices();
    if (param_page_) param_page_->refresh();

    main_box_.append(stack_switcher_);
    main_box_.append(stack_);
    main_box_.append(status_box_);

    set_titlebar(header_bar_);
    set_child(main_box_);
    update_status_bar();

    if (show_splash)
        create_splash_screen();
}

void MainWindow::create_splash_screen()
{
    auto splash = Gtk::make_managed<Gtk::Window>();
    splash->set_title("Servo_Tuning");
    splash->set_transient_for(*this);
    splash->set_modal(true);
    splash->set_default_size(420, 260);
    splash->set_resizable(false);

    auto splash_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 8);
    splash_box->set_margin(24);
    splash_box->set_halign(Gtk::Align::CENTER);
    splash_box->set_valign(Gtk::Align::CENTER);

    auto splash_css = Gtk::CssProvider::create();
    splash_css->load_from_data(
        "label.title { font-size: 20pt; font-weight: bold; }\n"
        "label.version { font-size: 12pt; font-weight: bold; }\n"
        "label.body { font-size: 13pt; font-weight: bold; }\n"
        "label.copyright { font-size: 12pt; font-weight: bold; }\n");
    splash->get_style_context()->add_provider(splash_css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    auto title_lbl = Gtk::make_managed<Gtk::Label>("Servo_Tuning");
    title_lbl->set_css_classes({"title"});

    auto ver_lbl = Gtk::make_managed<Gtk::Label>("1.0.0");
    ver_lbl->set_css_classes({"version"});

    auto comment_lbl = Gtk::make_managed<Gtk::Label>();
    comment_lbl->set_css_classes({"body"});
    comment_lbl->set_text(tr("about_comments"));
    comment_lbl->set_justify(Gtk::Justification::CENTER);

    auto copyright_lbl = Gtk::make_managed<Gtk::Label>();
    copyright_lbl->set_css_classes({"copyright"});
    copyright_lbl->set_text(tr("about_copyright"));

    splash_box->append(*title_lbl);
    splash_box->append(*ver_lbl);
    splash_box->append(*comment_lbl);
    splash_box->append(*copyright_lbl);
    splash->set_child(*splash_box);

    auto step = std::make_shared<int>(0);
    auto langs = std::make_shared<std::vector<const char*>>(
        std::vector<const char*>{"ru", "zh", "en"});
    auto alive = std::make_shared<bool>(true);

    // Splash cycles LANGUAGE for its rotating text (see timeout below).
    // Remember the real language so closing the splash restores it —
    // otherwise the whole app keeps whatever language the cycle stopped on.
    auto saved_lang = std::make_shared<std::string>([] {
        const char* l = getenv("LANGUAGE");
        return std::string(l ? l : "ru");
    }());
    splash->signal_destroy().connect([alive, comment_lbl, copyright_lbl, saved_lang]() {
        *alive = false;
        setenv("LANGUAGE", saved_lang->c_str(), 1);
    });

    splash->present();

    // Language cycling, stops if dialog is closed.
    // NOTE: must NOT setenv("LANGUAGE") — the main window is being built
    // concurrently and used to freeze whatever rotation step was active
    // (users saw a Chinese UI). Rotate the text via tr_in() instead.
    Glib::signal_timeout().connect([step, langs, alive, comment_lbl, copyright_lbl]() -> bool {
        if (!*alive) return false;
        (*step)++;
        if (*step >= 3) *step = 0;
        const std::string l = (*langs)[*step];
        comment_lbl->set_text(tr_in(l, "about_comments"));
        copyright_lbl->set_text(tr_in(l, "about_copyright"));
        return true;
    }, 2000);

    // Block close for first 6 seconds
    auto can_close = std::make_shared<bool>(false);
    splash->signal_close_request().connect([can_close]() -> bool {
        return !*can_close;
    }, true);
    Glib::signal_timeout().connect_once([can_close]() {
        *can_close = true;
    }, 6000);
}

void MainWindow::build_ui()
{
    main_box_.set_orientation(Gtk::Orientation::VERTICAL);

    stack_switcher_.set_stack(stack_);
    stack_switcher_.set_halign(Gtk::Align::CENTER);
    stack_switcher_.set_margin(6);

    param_page_ = std::make_unique<ParamPage>(*this, sdo_client_);
    monitor_page_ = std::make_unique<MonitorPage>(sdo_client_);

    stack_.add(*param_page_, "params", tr("stack_params"));
    stack_.add(*monitor_page_, "monitor", tr("stack_monitor"));

    stack_.set_vexpand(true);

    status_box_.set_orientation(Gtk::Orientation::HORIZONTAL);
    status_box_.set_spacing(12);
    status_box_.set_margin_start(12);
    status_box_.set_margin_end(12);
    status_box_.set_margin_top(4);
    status_box_.set_margin_bottom(4);

    // Status text
    status_label_.set_text(tr("status_bar"));
    status_label_.set_css_classes({"status-label"});
    status_box_.append(status_label_);

    // Device selector
    device_label_.set_text(tr("device"));
    device_label_.set_css_classes({"status-label"});

    scan_btn_.set_label(tr("btn_scan"));
    scan_btn_.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_scan_devices));

    device_combo_.set_hexpand(true);
    conn_device_changed_ = device_combo_.signal_changed().connect(
        sigc::mem_fun(*this, &MainWindow::on_device_changed));
    device_combo_.append("0: No device");
    device_combo_.set_active(0);

    status_box_.append(device_label_);
    status_box_.append(device_combo_);
    status_box_.append(scan_btn_);

    // Language selector
    lang_combo_.set_hexpand(true);
    lang_combo_.set_halign(Gtk::Align::END);
    lang_combo_.append("ru", tr("lang_ru"));
    lang_combo_.append("zh", tr("lang_zh"));
    lang_combo_.append("en", tr("lang_en"));

    const char* lang = getenv("LANGUAGE");
    std::string current = lang ? lang : "ru";
    if (current.size() > 2) current = current.substr(0, 2);
    lang_combo_.set_active_id(current);

    conn_lang_changed_ = lang_combo_.signal_changed().connect([this] { on_language_changed(); });

    status_box_.append(lang_combo_);
}

void MainWindow::build_menu()
{
    menu_button_.set_icon_name("open-menu-symbolic");
    menu_button_.set_tooltip_text(tr("menu_btn"));

    popover_.set_child(menu_box_);
    menu_box_.set_spacing(4);
    menu_box_.set_margin(8);

    btn_about_.set_label(tr("menu_about"));
    btn_about_.signal_clicked().connect([this] {
        auto dialog = Gtk::make_managed<Gtk::AboutDialog>();
        dialog->set_program_name("Servo_Tuning");
        dialog->set_version("1.0.0");
        dialog->set_comments(tr("about_comments"));
        dialog->set_copyright(tr("about_copyright"));
        dialog->set_website("mailto:warbirds@mail.ru");
        dialog->set_transient_for(*this);
        auto css = Gtk::CssProvider::create();
        css->load_from_data("label { font-size: 12pt; }");
        dialog->get_style_context()->add_provider(css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        dialog->present();
    });

    btn_quit_.set_label(tr("menu_quit"));
    btn_quit_.signal_clicked().connect([this] {
        close();
    });

    menu_box_.append(btn_about_);
    menu_box_.append(btn_quit_);
    menu_button_.set_popover(popover_);

    // Own window controls instead of CSD titlebuttons. Verified by three
    // probe builds: the Breeze GTK4 theme draws the close/minimize/maximize
    // glyphs as background-images on the titlebuttons and hides the image
    // nodes themselves — on our dark headerbar they render invisible and no
    // CSS override (-gtk-icon-source on button or image node) revives them.
    // Text glyphs are Pango-rendered and immune to all of this.
    header_bar_.set_show_title_buttons(false);

    btn_win_close_.set_label("✕");  // U+2715
    btn_win_close_.add_css_class("titlebtn");
    btn_win_close_.add_css_class("btn-win-close");
    btn_win_close_.signal_clicked().connect([this] { close(); });

    btn_win_max_.set_label("□");    // U+25A1
    btn_win_max_.add_css_class("titlebtn");
    btn_win_max_.signal_clicked().connect([this] {
        if (gtk_window_is_maximized(GTK_WINDOW(gobj())))
            unmaximize();
        else
            maximize();
    });

    btn_win_min_.set_label("–");    // U+2013
    btn_win_min_.add_css_class("titlebtn");
    btn_win_min_.signal_clicked().connect([this] { minimize(); });

    // pack_end stacks from the right edge: close, max, min, then menu
    header_bar_.pack_end(btn_win_close_);
    header_bar_.pack_end(btn_win_max_);
    header_bar_.pack_end(btn_win_min_);
    header_bar_.pack_end(menu_button_);
}

void MainWindow::on_language_changed()
{
    conn_lang_changed_.block();  // Prevent recursion

    std::string selected = lang_combo_.get_active_id();
    if (selected.empty()) selected = "ru";

    setenv("LANGUAGE", selected.c_str(), 1);

    const char* home = getenv("HOME");
    if (!home) home = "/tmp";
    std::string dir = std::string(home) + "/.config/servo_tuning";
    std::filesystem::create_directories(dir);
    std::ofstream f(dir + "/settings.json");
    if (f.is_open()) {
        f << "{\n  \"lang\": \"" << selected << "\"\n}\n";
        f.close();
    }

    lang_combo_.remove_all();
    lang_combo_.append("ru", tr("lang_ru"));
    lang_combo_.append("zh", tr("lang_zh"));
    lang_combo_.append("en", tr("lang_en"));

    lang_combo_.set_active_id(selected);

    conn_lang_changed_.unblock();  // Restore signal

    // Update Gtk::StackPage titles
    Glib::RefPtr<Gtk::StackPage> param_page = stack_.get_page(*param_page_);
    if (param_page) param_page->set_title(tr("stack_params"));

    Glib::RefPtr<Gtk::StackPage> monitor_page = stack_.get_page(*monitor_page_);
    if (monitor_page) monitor_page->set_title(tr("stack_monitor"));

    param_page_->refresh();
    monitor_page_->refresh();

    update_status_bar();
    device_label_.set_text(tr("device"));
    scan_btn_.set_label(tr("btn_scan"));
    btn_about_.set_label(tr("menu_about"));
    btn_quit_.set_label(tr("menu_quit"));
    menu_button_.set_tooltip_text(tr("menu_btn"));
    set_title(tr("window_title"));
}

void MainWindow::update_status_bar()
{
    status_label_.set_text(tr("status_bar"));
    status_label_.set_css_classes({"status-label"});
}

void MainWindow::on_scan_devices()
{
    if (scanning_) return;
    scanning_ = true;
    scan_btn_.set_sensitive(false);
    scan_preserve_id_ = device_combo_.get_active_id();

    // The `ethercat slaves` CLI blocks on a dead bus: run it on a worker
    // thread so the main loop is never frozen (CLAUDE.md threading rule)
    scan_thread_ = std::thread([this] {
        scan_results_.clear();
        // timeout: the CLI must not be able to wedge the worker on a dead bus
        FILE* pipe = popen("timeout -k 1 3 ethercat slaves 2>/dev/null", "r");
        if (pipe) {
            char buf[256];
            while (fgets(buf, sizeof(buf), pipe)) {
                std::string line(buf);
                while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
                    line.pop_back();

                if (line.empty()) continue;

                auto space = line.find(' ');
                if (space == std::string::npos) continue;
                std::string pos_str = line.substr(0, space);
                char* end = nullptr;
                long pos = strtol(pos_str.c_str(), &end, 10);
                if (end == pos_str.c_str()) continue;

                scan_results_.emplace_back(std::to_string(pos), line);
            }
            pclose(pipe);
        }
        scan_done_.emit();
    });
}

void MainWindow::apply_scan_results()
{
    if (scan_thread_.joinable())
        scan_thread_.join();
    scanning_ = false;
    scan_btn_.set_sensitive(true);

    conn_device_changed_.block();
    device_combo_.remove_all();
    for (auto& [id, label] : scan_results_)
        device_combo_.append(id, label);

    if (scan_results_.empty()) {
        device_combo_.append("0: No device", "0: No device");
        device_combo_.set_active(0);
    } else {
        if (!scan_preserve_id_.empty())
            device_combo_.set_active_id(scan_preserve_id_);
        if (device_combo_.get_active_row_number() < 0)
            device_combo_.set_active(0);
    }

    conn_device_changed_.unblock();
}

void MainWindow::on_device_changed()
{
    Glib::ustring id = device_combo_.get_active_id();
    if (id.empty()) return;

    char* end = nullptr;
    long pos = strtol(id.c_str(), &end, 10);
    if (end == id.c_str()) return;

#ifdef HAVE_ETHERCAT_H
    if (ecrt_domain_.is_active()) {
        ecrt_domain_.close();
    }
#endif

    sdo_client_.reset(static_cast<uint16_t>(pos));
    sdo_client_.connect();

    // Note: ecrt domain is not opened — CLI backend is used for SDO operations
    // ecrt_request_master() cannot work in background threads and causes I/O errors
    param_page_->refresh();
    update_status_bar();
}


