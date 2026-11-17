#pragma once
#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/columnview.h>
#include <gtkmm/entry.h>
#include <gtkmm/dropdown.h>
#include "param_row.h"
#include "sdo_client.h"
#include "param_db.h"
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <thread>

class ParamPage : public Gtk::Box {
public:
    ParamPage(Gtk::Window& parent, SdoClient& sdo);
    ~ParamPage() override = default;

public:
    void refresh();

private:
    void build_ui();
    void on_filter_changed();
    void populate_sample_data();
    void on_refresh_current();
    void on_row_activated(guint position);

    Gtk::Box top_bar_;
    Gtk::Entry search_entry_;
    Gtk::Button top_refresh_btn_;
    Gtk::DropDown group_selector_;
    Gtk::ScrolledWindow scroll_;
    Gtk::ColumnView column_view_;
    Glib::RefPtr<Gio::ListStore<ParamRow>> param_store_;
    Glib::RefPtr<Gtk::SingleSelection> selection_model_;
    Glib::RefPtr<Gtk::StringList> group_model_;
    std::vector<Glib::RefPtr<Gtk::ColumnViewColumn>> columns_;

    Gtk::Window* parent_window_ = nullptr;
    SdoClient& sdo_client_;
    Glib::Dispatcher dispatcher_;
    std::atomic<bool> reading_{false};
    std::mutex mutex_;
    std::thread refresh_thread_;
    std::map<std::string, std::string> pending_results_;
    std::map<std::string, std::string> cached_values_;
};
