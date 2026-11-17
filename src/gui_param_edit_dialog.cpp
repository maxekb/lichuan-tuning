#include "gui_param_edit_dialog.h"
#include "i18n.h"
#include <glibmm/markup.h>
#include <sstream>
#include <iomanip>
#include <cmath>

ParamEditDialog::ParamEditDialog(
    Gtk::Window& parent,
    const ParamDef& def,
    const Glib::ustring& current_val,
    SdoClient& sdo_client,
    std::function<void(const Glib::ustring&)> on_save
)
    : Gtk::Window()
    , def_(def)
    , sdo_client_(sdo_client)
    , on_save_(std::move(on_save))
{
    save_done_.connect([this] { on_save_done(); });
    set_modal(true);
    set_transient_for(parent);
    set_title(Glib::ustring(std::string(def_.name)));
    set_default_size(400, 280);

    double scale = def_.scale_factor();
    try {
        current_val_ = std::stod(std::string(current_val));
    } catch (...) {
        current_val_ = def_.default_val;
    }
    current_val_ = std::round(current_val_ * scale) / scale;
    current_val_ = std::clamp(current_val_, def_.min_val, def_.max_val);

    build_ui();
}

ParamEditDialog::~ParamEditDialog()
{
    if (save_thread_.joinable())
        save_thread_.join();
}

void ParamEditDialog::build_ui()
{
    main_vbox_.set_orientation(Gtk::Orientation::VERTICAL);
    main_vbox_.set_spacing(12);
    main_vbox_.set_margin(20);

    title_label_.set_text(Glib::ustring(std::string(def_.name)));
    title_label_.set_css_classes({"edit-dialog-title"});
    title_label_.set_halign(Gtk::Align::CENTER);
    main_vbox_.append(title_label_);

    button_row_.set_orientation(Gtk::Orientation::HORIZONTAL);
    button_row_.set_spacing(8);
    button_row_.set_halign(Gtk::Align::CENTER);
    button_row_.set_homogeneous(false);

    btn_dec10_.set_tooltip_text("-10");
    btn_dec10_.set_label("\xe2\x97\x80\xe2\x97\x80");
    btn_dec10_.set_css_classes({"edit-btn"});
    btn_dec10_.signal_clicked().connect([this] { adjust_value(-10); });

    btn_dec1_.set_tooltip_text("-1");
    btn_dec1_.set_label("\xe2\x97\x80");
    btn_dec1_.set_css_classes({"edit-btn"});
    btn_dec1_.signal_clicked().connect([this] { adjust_value(-1); });

    value_label_.set_css_classes({"edit-value"});
    value_label_.set_halign(Gtk::Align::CENTER);
    value_label_.set_hexpand(true);

    {
        std::string unit_key = std::string(def_.name) + std::string("_unit");
        std::string unit_text = tr(unit_key);
        if (unit_text == unit_key) unit_text = std::string(def_.unit);
        unit_label_.set_text(Glib::ustring(unit_text));
    }
    unit_label_.set_css_classes({"edit-unit"});
    unit_label_.set_halign(Gtk::Align::START);

    btn_inc1_.set_tooltip_text("+1");
    btn_inc1_.set_label("\xe2\x96\xb6");
    btn_inc1_.set_css_classes({"edit-btn"});
    btn_inc1_.signal_clicked().connect([this] { adjust_value(1); });

    btn_inc10_.set_tooltip_text("+10");
    btn_inc10_.set_label("\xe2\x96\xb6\xe2\x96\xb6");
    btn_inc10_.set_css_classes({"edit-btn"});
    btn_inc10_.signal_clicked().connect([this] { adjust_value(10); });

    button_row_.append(btn_dec10_);
    button_row_.append(btn_dec1_);
    button_row_.append(value_label_);
    button_row_.append(unit_label_);
    button_row_.append(btn_inc1_);
    button_row_.append(btn_inc10_);
    main_vbox_.append(button_row_);

    update_value_display();

    int prec = def_.decimal_places;
    if (prec < 0) prec = 2;
    std::ostringstream rs;
    rs << std::fixed << std::setprecision(prec) << def_.min_val
       << " \xe2\x80\x93 " << std::fixed << std::setprecision(prec) << def_.max_val
       << "  " << def_.unit;
    range_label_.set_text(tr("dialog_range") + " " + rs.str());
    range_label_.set_css_classes({"edit-range"});
    range_label_.set_halign(Gtk::Align::CENTER);
    main_vbox_.append(range_label_);

    if (!def_.value_descs.empty()) {
        auto vd_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        vd_box->set_spacing(2);
        vd_box->set_margin(4);
        vd_box->set_css_classes({"edit-value-descs"});
        for (auto& [val, desc] : def_.value_descs) {
            std::string translated = tr(desc);
            if (translated == desc) translated = desc;
            auto vl = Gtk::make_managed<Gtk::Label>(Glib::ustring::sprintf("%lld: %s", (long long)val, translated.c_str()));
            vl->set_css_classes({"edit-value-desc-item"});
            vd_box->append(*vl);
        }
        main_vbox_.insert_child_after(*vd_box, range_label_);
    }

    std::string desc_key = std::string(def_.name) + std::string("_desc");
    std::string desc_text = tr(desc_key);
    if (desc_text == desc_key) desc_text = std::string(def_.desc);
    desc_label_.set_text(Glib::ustring(desc_text));
    desc_label_.set_css_classes({"edit-desc"});
    desc_label_.set_halign(Gtk::Align::CENTER);
    main_vbox_.append(desc_label_);

    error_label_.set_css_classes({"edit-error"});
    error_label_.set_halign(Gtk::Align::CENTER);
    error_label_.set_visible(false);
    main_vbox_.append(error_label_);

    action_row_.set_orientation(Gtk::Orientation::HORIZONTAL);
    action_row_.set_spacing(12);
    action_row_.set_halign(Gtk::Align::CENTER);
    action_row_.set_margin_top(12);

    save_btn_.set_label(tr("btn_save"));
    save_btn_.set_css_classes({"btn-primary"});
    save_btn_.signal_clicked().connect(
        sigc::mem_fun(*this, &ParamEditDialog::on_save_clicked));

    cancel_btn_.set_label(tr("btn_cancel"));
    cancel_btn_.signal_clicked().connect(
        sigc::mem_fun(*this, &ParamEditDialog::on_cancel_clicked));

    action_row_.append(save_btn_);
    action_row_.append(cancel_btn_);
    main_vbox_.append(action_row_);

    if (def_.apply == ApplyMode::READ_ONLY) {
        btn_dec10_.set_sensitive(false);
        btn_dec1_.set_sensitive(false);
        btn_inc1_.set_sensitive(false);
        btn_inc10_.set_sensitive(false);
        save_btn_.set_sensitive(false);
        save_btn_.set_label(tr("read_only"));
    }

    set_child(main_vbox_);
}

void ParamEditDialog::adjust_value(int factor)
{
    double scale = def_.scale_factor();
    double step = 1.0;
    if (scale > 1.0) step = 1.0 / scale;
    current_val_ += step * factor;
    current_val_ = std::round(current_val_ * scale) / scale;
    current_val_ = std::clamp(current_val_, def_.min_val, def_.max_val);
    update_value_display();
}

void ParamEditDialog::update_value_display()
{
    int prec = def_.decimal_places;
    if (prec < 0) prec = 2;
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(prec) << current_val_;
    value_label_.set_text(ss.str());
    {
        std::string unit_key = std::string(def_.name) + std::string("_unit");
        std::string unit_text = tr(unit_key);
        if (unit_text == unit_key) unit_text = std::string(def_.unit);
        unit_label_.set_text(Glib::ustring(unit_text));
    }
}

void ParamEditDialog::on_save_clicked()
{
    if (def_.apply == ApplyMode::READ_ONLY) return;

    // Range validation
    if (current_val_ < def_.min_val || current_val_ > def_.max_val) {
        fprintf(stderr, "[ParamEdit] Value %.6f out of range [%.6f, %.6f]\n",
            current_val_, def_.min_val, def_.max_val);
        return;
    }

    double scale = def_.scale_factor();
    int64_t raw = static_cast<int64_t>(current_val_ * scale);

    // Validate raw value against parameter definition range
    if (!def_.validate_raw(raw)) {
        fprintf(stderr, "[ParamEdit] Raw value %lld invalid for %s\n",
            static_cast<long long>(raw), std::string(def_.name).c_str());
        return;
    }

    std::vector<uint8_t> bytes;
    switch (def_.type) {
        case ParamType::UINT8:
        case ParamType::INT8:
            bytes.push_back(static_cast<uint8_t>(raw & 0xFF));
            break;
        case ParamType::UINT16:
        case ParamType::INT16:
            bytes.push_back(static_cast<uint8_t>(raw & 0xFF));
            bytes.push_back(static_cast<uint8_t>((raw >> 8) & 0xFF));
            break;
        case ParamType::UINT32:
        case ParamType::INT32:
        case ParamType::FLOAT:
            bytes.push_back(static_cast<uint8_t>(raw & 0xFF));
            bytes.push_back(static_cast<uint8_t>((raw >> 8) & 0xFF));
            bytes.push_back(static_cast<uint8_t>((raw >> 16) & 0xFF));
            bytes.push_back(static_cast<uint8_t>((raw >> 24) & 0xFF));
            break;
        default:
            return;
    }

    SdoType sdo_type;
    switch (def_.type) {
        case ParamType::UINT8:  sdo_type = SdoType::UINT8;  break;
        case ParamType::INT8:   sdo_type = SdoType::INT8;   break;
        case ParamType::UINT16: sdo_type = SdoType::UINT16; break;
        case ParamType::INT16:  sdo_type = SdoType::INT16;  break;
        case ParamType::UINT32: sdo_type = SdoType::UINT32; break;
        case ParamType::INT32:  sdo_type = SdoType::INT32;  break;
        case ParamType::FLOAT:  sdo_type = SdoType::FLOAT;  break;
        default: return;
    }

    auto do_save = [this, bytes, sdo_type]() {
        if (saving_) return;
        saving_ = true;
        save_btn_.set_sensitive(false);
        // The device receives the bytes captured above, so +/- must stay
        // insensitive while the write is in flight: a mutated current_val_
        // must never be reported back as the value the device holds.
        btn_dec10_.set_sensitive(false);
        btn_dec1_.set_sensitive(false);
        btn_inc1_.set_sensitive(false);
        btn_inc10_.set_sensitive(false);
        // Snapshot the display value that corresponds to the sent bytes
        int prec = def_.decimal_places;
        if (prec < 0) prec = 2;
        std::ostringstream snap;
        snap << std::fixed << std::setprecision(prec) << current_val_;
        saved_display_ = snap.str();
        error_label_.set_text("");
        error_label_.set_visible(false);
        save_thread_ = std::thread([this, bytes, sdo_type]() {
            uint32_t abort_code = 0;
            const bool ok = sdo_client_.write(def_.index, def_.subindex, bytes, sdo_type, &abort_code);
            save_abort_.store(abort_code);
            save_ok_.store(ok);
            save_done_.emit();
        });
    };

    // Confirm dangerous writes
    if (def_.index == 0x200D && def_.subindex == 0x00) {
        auto alert = Gtk::AlertDialog::create(tr("confirm_p0d00"));
        alert->set_message(tr("confirm_title"));
        alert->set_detail(tr("confirm_p0d00_msg"));
        alert->set_buttons({tr("btn_cancel"), tr("btn_save")});
        alert->set_cancel_button(0);
        alert->set_default_button(1);
        Glib::RefPtr<Gtk::AlertDialog> alert_copy = alert;
        alert->choose(*this, [alert_copy, do_save](Glib::RefPtr<Gio::AsyncResult> result) {
            int response = alert_copy->choose_finish(result);
            if (response == 1)
                do_save();
        });
        return;
    }
    if (def_.index == 0x200D && def_.subindex == 0x01) {
        auto alert = Gtk::AlertDialog::create(tr("confirm_p0d01"));
        alert->set_message(tr("confirm_title"));
        alert->set_detail(tr("confirm_p0d01_msg"));
        alert->set_buttons({tr("btn_cancel"), tr("btn_save")});
        alert->set_cancel_button(0);
        alert->set_default_button(1);
        Glib::RefPtr<Gtk::AlertDialog> alert_copy = alert;
        alert->choose(*this, [alert_copy, do_save](Glib::RefPtr<Gio::AsyncResult> result) {
            int response = alert_copy->choose_finish(result);
            if (response == 1)
                do_save();
        });
        return;
    }
    if (def_.index == 0x200D && def_.subindex == 0x05) {
        auto alert = Gtk::AlertDialog::create(tr("confirm_p0d05"));
        alert->set_message(tr("confirm_title"));
        alert->set_detail(tr("confirm_p0d05_msg"));
        alert->set_buttons({tr("btn_cancel"), tr("btn_save")});
        alert->set_cancel_button(0);
        alert->set_default_button(1);
        Glib::RefPtr<Gtk::AlertDialog> alert_copy = alert;
        alert->choose(*this, [alert_copy, do_save](Glib::RefPtr<Gio::AsyncResult> result) {
            int response = alert_copy->choose_finish(result);
            if (response == 1)
                do_save();
        });
        return;
    }

    do_save();
}

void ParamEditDialog::on_save_done()
{
    if (save_thread_.joinable())
        save_thread_.join();
    saving_ = false;

    if (!save_ok_.load()) {
        if (save_abort_.load())
            fprintf(stderr, "[ParamEdit] SDO write failed: index=0x%04X sub=0x%02X abort=0x%x\n",
                def_.index, def_.subindex, save_abort_.load());
        else
            fprintf(stderr, "[ParamEdit] SDO write failed: index=0x%04X sub=0x%02X (no abort code; see [SdoClient] log)\n",
                def_.index, def_.subindex);
        // Errors go to error_label_ — the parameter description must survive
        gchar* esc = g_markup_escape_text(tr("write_failed").c_str(), -1);
        error_label_.set_markup(
            Glib::ustring("<span foreground=\"#e05555\">") + esc + "</span>");
        g_free(esc);
        error_label_.set_visible(true);
        save_btn_.set_sensitive(true);
        btn_dec10_.set_sensitive(true);
        btn_dec1_.set_sensitive(true);
        btn_inc1_.set_sensitive(true);
        btn_inc10_.set_sensitive(true);
        return;
    }

    if (on_save_) on_save_(saved_display_);

    close();
}

void ParamEditDialog::on_cancel_clicked()
{
    close();
}
