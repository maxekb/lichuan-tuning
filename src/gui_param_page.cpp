#include "gui_param_page.h"
#include "gui_param_edit_dialog.h"
#include "param_db.h"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include "i18n.h"

ParamPage::ParamPage(Gtk::Window& parent, SdoClient& sdo)
    : parent_window_(&parent)
    , sdo_client_(sdo)
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_spacing(6);
    set_margin(8);
    build_ui();

    dispatcher_.connect([this] {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            cached_values_.insert(pending_results_.begin(), pending_results_.end());
            pending_results_.clear();
        }
        populate_sample_data();
        top_refresh_btn_.set_sensitive(true);
        reading_ = false;
    });
}

static Glib::RefPtr<Gtk::SignalListItemFactory> make_factory(
    std::function<Glib::ustring(const Glib::RefPtr<ParamRow>&)> getter,
    Gtk::Align align = Gtk::Align::START)
{
    auto factory = Gtk::SignalListItemFactory::create();
    factory->signal_setup().connect([align](const Glib::RefPtr<Gtk::ListItem>& item) {
        auto label = Gtk::make_managed<Gtk::Label>("", Gtk::Align::START);
        label->set_halign(align);
        label->set_css_classes({"cell-label"});
        item->set_child(*label);
    });
    factory->signal_bind().connect([getter](const Glib::RefPtr<Gtk::ListItem>& item) {
        auto label = dynamic_cast<Gtk::Label*>(item->get_child());
        if (!label) return;
        auto row = std::dynamic_pointer_cast<ParamRow>(item->get_item());
        if (!row) return;
        label->set_text(getter(row));

        auto* def = param_db().find(std::string(row->id));
        bool editable = def && def->apply != ApplyMode::READ_ONLY;
        label->set_opacity(editable ? 1.0 : 0.4);
    });
    return factory;
}

void ParamPage::build_ui()
{
    top_bar_.set_orientation(Gtk::Orientation::HORIZONTAL);
    top_bar_.set_spacing(8);
    top_bar_.set_margin_bottom(6);

    top_refresh_btn_.set_label("\xe2\x9f\xb3");
    top_refresh_btn_.set_tooltip_text(tr("btn_refresh"));
    top_refresh_btn_.signal_clicked().connect(
        sigc::mem_fun(*this, &ParamPage::on_refresh_current));
    top_bar_.append(top_refresh_btn_);

    search_entry_.set_placeholder_text(tr("search_param"));
    search_entry_.set_hexpand(true);
    search_entry_.signal_changed().connect(
        sigc::mem_fun(*this, &ParamPage::on_filter_changed));

    group_model_ = Gtk::StringList::create({
        tr("group_all"), tr("group_p01"), tr("group_p02"), tr("group_p03"),
        tr("group_p04"), tr("group_p05"), tr("group_p06"), tr("group_p07"),
        tr("group_p08"), tr("group_p09"), tr("group_p0a"), tr("group_p0b"),
        tr("group_p0c"), tr("group_p0d")
    });
    group_selector_.set_model(group_model_);
    group_selector_.set_selected(0);
    group_selector_.property_selected().signal_changed().connect(
        sigc::mem_fun(*this, &ParamPage::on_filter_changed));

    top_bar_.append(search_entry_);
    top_bar_.append(group_selector_);
    append(top_bar_);

    param_store_ = Gio::ListStore<ParamRow>::create();
    selection_model_ = Gtk::SingleSelection::create(param_store_);
    column_view_.set_model(selection_model_);

    auto add_column = [&](const Glib::ustring& title, int flex, Gtk::Align align,
                          std::function<Glib::ustring(const Glib::RefPtr<ParamRow>&)> getter) {
        auto col = Gtk::ColumnViewColumn::create(title, make_factory(getter, align));
        col->set_resizable(true);
        if (flex > 0) col->set_expand(true);
        column_view_.append_column(col);
        columns_.push_back(col);
    };

    add_column(tr("col_param"),     0, Gtk::Align::START, [](auto& r) { return r->id; });
    columns_.back()->set_fixed_width(100);

    {  // Current column
        auto curr_factory = Gtk::SignalListItemFactory::create();
        curr_factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem>& item) {
            auto label = Gtk::make_managed<Gtk::Label>();
            label->set_css_classes({"param-current", "cell-label"});
            label->set_halign(Gtk::Align::CENTER);
            label->set_hexpand(true);
            item->set_child(*label);
        });
        curr_factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem>& item) {
            auto label = dynamic_cast<Gtk::Label*>(item->get_child());
            if (!label) return;
            auto row = std::dynamic_pointer_cast<ParamRow>(item->get_item());
            if (!row) return;
            label->set_text(row->current);
            label->set_tooltip_text(row->unit);

            auto* def = param_db().find(std::string(row->id));
            bool editable = def && def->apply != ApplyMode::READ_ONLY;
            label->set_opacity(editable ? 1.0 : 0.4);
        });
        auto curr_col = Gtk::ColumnViewColumn::create(tr("col_current"), curr_factory);
        curr_col->set_resizable(true);
        curr_col->set_fixed_width(110);
        column_view_.append_column(curr_col);
        columns_.push_back(curr_col);
    }
    add_column(tr("col_default"),   0, Gtk::Align::CENTER, [](auto& r) { return r->default_val; });
    columns_.back()->set_fixed_width(90);

    add_column(tr("col_range"),     1, Gtk::Align::CENTER, [](auto& r) { return r->range; });
    columns_.back()->set_fixed_width(180);

    add_column(tr("col_unit"),      0, Gtk::Align::CENTER, [](auto& r) { return r->unit; });
    columns_.back()->set_fixed_width(70);

    add_column(tr("col_desc"),      1, Gtk::Align::START, [](auto& r) { return r->desc; });

    // Double-click to edit
    auto gesture = Gtk::GestureClick::create();
    gesture->set_button(1);
    gesture->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
    gesture->signal_pressed().connect([this](int n_press, double, double) {
        if (n_press == 2) {
            auto selected = selection_model_->get_selected();
            if (selected != GTK_INVALID_LIST_POSITION)
                on_row_activated(selected);
        }
    });
    column_view_.add_controller(gesture);

    scroll_.set_child(column_view_);
    scroll_.set_vexpand(true);
    append(scroll_);

    populate_sample_data();
}

void ParamPage::populate_sample_data()
{
    param_store_->remove_all();

    auto passes_filter = [&](const Glib::RefPtr<ParamRow>& row) -> bool {
        guint group_idx = group_selector_.get_selected();
        if (group_idx > 0) {
            std::ostringstream prefix;
            prefix << "P0" << std::hex << std::uppercase << group_idx;
            std::string pf = prefix.str();
            if (row->id.size() < pf.size() || row->id.substr(0, pf.size()) != pf)
                return false;
        }

        Glib::ustring search = search_entry_.get_text();
        if (!search.empty()) {
            auto ci_find = [&](const Glib::ustring& s) {
                return s.lowercase().find(search.lowercase()) != Glib::ustring::npos;
            };
            if (!ci_find(row->id) && !ci_find(row->desc) && !ci_find(row->unit))
                return false;
        }

        return true;
    };

    for (const auto& def : param_db().all()) {
        int prec = def.decimal_places;
        if (prec < 0) prec = 2;
        auto fmt = [&](double v) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(prec) << v;
            return ss.str();
        };

        Glib::ustring dv = fmt(def.default_val);
        Glib::ustring rn = fmt(def.min_val) + " \342\200\223 " + fmt(def.max_val);

        std::string desc_key = std::string(def.name) + "_desc";
        std::string desc_text = tr(desc_key);
        if (desc_text == desc_key) desc_text = std::string(def.desc);

        auto cit = cached_values_.find(std::string(def.name));
        if (!def.value_descs.empty() && cit != cached_values_.end() && cit->second != "\xe2\x80\x94" && cit->second != "ERR") {
            try {
                int64_t val = static_cast<int64_t>(std::stod(cit->second));
                auto vit = def.value_descs.find(val);
                if (vit != def.value_descs.end()) {
                    std::string desc = tr(vit->second);
                    if (desc == vit->second) desc = vit->second;
                    desc_text += " [" + Glib::ustring(desc) + "]";
                }
            } catch (...) {}
        }

        std::string unit_key = std::string(def.name) + "_unit";
        std::string unit_text = tr(unit_key);
        if (unit_text == unit_key) unit_text = std::string(def.unit);

        auto row = ParamRow::create(
            Glib::ustring(std::string(def.name)),
            [&]() -> Glib::ustring {
                auto it = cached_values_.find(std::string(def.name));
                if (it != cached_values_.end()) {
                    if (it->second != dv && it->second != "\xe2\x80\x94" && it->second != "ERR")
                        return it->second + " *";
                    return it->second;
                }
                return "\xe2\x80\x94";
            }(),
            dv,
            rn,
            Glib::ustring(unit_text),
            Glib::ustring(desc_text)
        );

        if (passes_filter(row))
            param_store_->append(row);
    }
}

void ParamPage::on_filter_changed()
{
    populate_sample_data();
}

void ParamPage::on_refresh_current()
{
    if (reading_) return;
    reading_ = true;
    top_refresh_btn_.set_sensitive(false);

    std::vector<const ParamDef*> to_read;
    for (guint i = 0; i < param_store_->get_n_items(); ++i) {
        auto row = param_store_->get_item(i);
        if (row) {
            auto def = param_db().find(std::string(row->id));
            if (def) to_read.push_back(def);
        }
    }

    if (refresh_thread_.joinable())
        refresh_thread_.join();
    refresh_thread_ = std::thread([this, to_read]() {
        std::map<std::string, std::string> results;
        for (auto* def : to_read) {
            if (def->type == ParamType::STRING) {
                results[std::string(def->name)] = "\xe2\x80\x94";
                continue;
            }

            SdoType sdo_type;
            switch (def->type) {
                case ParamType::UINT8:  sdo_type = SdoType::UINT8;  break;
                case ParamType::INT8:   sdo_type = SdoType::INT8;   break;
                case ParamType::UINT16: sdo_type = SdoType::UINT16; break;
                case ParamType::INT16:  sdo_type = SdoType::INT16;  break;
                case ParamType::UINT32: sdo_type = SdoType::UINT32; break;
                case ParamType::INT32:  sdo_type = SdoType::INT32;  break;
                case ParamType::FLOAT:  sdo_type = SdoType::FLOAT;  break;
                default: continue;
            }

            auto sdo_result = sdo_client_.read(def->index, def->subindex, sdo_type);
            if (!sdo_result.success) {
                results[std::string(def->name)] = "ERR";
                continue;
            }

            uint64_t val = 0;
            for (size_t i = 0; i < sdo_result.data.size() && i < 8; ++i)
                val |= static_cast<uint64_t>(sdo_result.data[i]) << (i * 8);

            int64_t raw_val;
            switch (def->type) {
                case ParamType::INT8:
                    raw_val = static_cast<int64_t>(static_cast<int8_t>(val & 0xFF));
                    break;
                case ParamType::INT16:
                    raw_val = static_cast<int64_t>(static_cast<int16_t>(val & 0xFFFF));
                    break;
                case ParamType::INT32:
                    raw_val = static_cast<int64_t>(static_cast<int32_t>(val & 0xFFFFFFFF));
                    break;
                default:
                    raw_val = static_cast<int64_t>(val);
                    break;
            }

            double display_val = raw_val / def->scale_factor();
            int prec = def->decimal_places;
            if (prec < 0) prec = 2;
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(prec) << display_val;
            results[std::string(def->name)] = ss.str();
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_results_ = std::move(results);
        }
        dispatcher_.emit();
    });
}

void ParamPage::on_row_activated(guint position)
{
    auto row = param_store_->get_item(position);
    if (!row) return;

    auto* def = param_db().find(std::string(row->id));
    if (!def || def->type == ParamType::STRING) return;

    auto* dialog = Gtk::make_managed<ParamEditDialog>(
        *parent_window_,
        *def,
        row->current,
        sdo_client_,
        [this, row_id = Glib::ustring(row->id)](const Glib::ustring& new_val) {
            cached_values_[row_id] = new_val;
            for (guint i = 0; i < param_store_->get_n_items(); i++) {
                auto r = param_store_->get_item(i);
                if (r && r->id == row_id) {
                    r->current = new_val;
                    std::vector<Glib::RefPtr<ParamRow>> items = { r };
                    param_store_->splice(i, 1, items);
                    break;
                }
            }
            // Re-read every parameter from the drive after a save: written
            // values may need a mode transition to take effect (ApplyMode),
            // so only an actual read shows the true state of the whole table.
            // on_refresh_current is worker-thread based with a reading_ guard
            // — safe to call from the dialog's main-thread save callback.
            on_refresh_current();
        }
    );
    dialog->set_visible(true);
}

void ParamPage::refresh()
{
    if (columns_.size() >= 6) {
        columns_[0]->set_title(tr("col_param"));
        columns_[1]->set_title(tr("col_current"));
        columns_[2]->set_title(tr("col_default"));
        columns_[3]->set_title(tr("col_range"));
        columns_[4]->set_title(tr("col_unit"));
        columns_[5]->set_title(tr("col_desc"));
    }

    if (group_model_) {
        std::vector<Glib::ustring> strings = {
            tr("group_all"), tr("group_p01"), tr("group_p02"), tr("group_p03"),
            tr("group_p04"), tr("group_p05"), tr("group_p06"), tr("group_p07"),
            tr("group_p08"), tr("group_p09"), tr("group_p0a"), tr("group_p0b"),
            tr("group_p0c"), tr("group_p0d")
        };
        group_model_->splice(0, group_model_->get_n_items(), strings);
    }

    search_entry_.set_placeholder_text(tr("search_param"));

    top_refresh_btn_.set_tooltip_text(tr("btn_refresh"));

    populate_sample_data();
}
