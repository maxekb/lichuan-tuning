#include "gui_graph.h"
#include "i18n.h"
#include <cmath>
#include <algorithm>

GraphWidget::GraphWidget()
{
    set_content_width(400);
    set_content_height(200);
    set_hexpand(true);
    set_vexpand(true);

    set_draw_func(sigc::mem_fun(*this, &GraphWidget::on_draw));
}

void GraphWidget::add_series(const Glib::ustring& name, const Gdk::RGBA& color,
                              double min_val, double max_val)
{
    std::lock_guard<std::mutex> lock(mutex_);
    series_.push_back({{}, color, min_val, max_val, name});
    Glib::signal_idle().connect_once([this] { queue_draw(); });
}

void GraphWidget::append_data(size_t series_idx, double value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (series_idx >= series_.size()) return;

    auto& s = series_[series_idx];
    s.data.push_back(value);

    size_t max_samples = static_cast<size_t>(time_window_ * sample_rate_);
    while (s.data.size() > max_samples)
        s.data.pop_front();

    Glib::signal_idle().connect_once([this] { queue_draw(); });
}

void GraphWidget::set_time_window(double seconds)
{
    time_window_ = seconds;
    queue_draw();
}

void GraphWidget::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& s : series_)
        s.data.clear();
    Glib::signal_idle().connect_once([this] { queue_draw(); });
}

void GraphWidget::set_series_name(size_t series_idx, const Glib::ustring& name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (series_idx < series_.size())
        series_[series_idx].name = name;
    Glib::signal_idle().connect_once([this] { queue_draw(); });
}

void GraphWidget::add_marker(const GraphMarker& marker)
{
    std::lock_guard<std::mutex> lock(mutex_);
    markers_.push_back(marker);
    Glib::signal_idle().connect_once([this] { queue_draw(); });
}

void GraphWidget::clear_markers()
{
    std::lock_guard<std::mutex> lock(mutex_);
    markers_.clear();
    Glib::signal_idle().connect_once([this] { queue_draw(); });
}

void GraphWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h)
{
    if (w <= 0 || h <= 0) return;

    cr->save();

    cr->set_source_rgb(0.12, 0.12, 0.13);
    cr->paint();

    const double margin_left = 60.0;
    const double margin_right = 20.0;
    const double margin_top = 10.0;
    const double margin_bottom = 30.0;
    const double plot_w = w - margin_left - margin_right;
    const double plot_h = h - margin_top - margin_bottom;

    if (plot_w <= 0 || plot_h <= 0) {
        cr->restore();
        return;
    }

    cr->translate(margin_left, margin_top);

    cr->rectangle(0, 0, plot_w, plot_h);
    cr->set_source_rgb(0.15, 0.15, 0.16);
    cr->fill();

    draw_grid(cr, plot_w, plot_h);
    draw_series(cr, plot_w, plot_h);
    draw_markers(cr, plot_w, plot_h);
    draw_legend(cr, plot_w, plot_h);

    // Axis labels — snapshot under lock, draw without lock
    GraphSeries snapshot;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!series_.empty())
            snapshot = series_[0];
    }
    if (snapshot.data.empty()) {
        cr->restore();
        return;
    }

    double rng = snapshot.max_val - snapshot.min_val;
    if (rng < 1e-10) rng = 1.0;

    for (int i = 0; i <= 4; ++i) {
        double val = snapshot.max_val - (rng * i / 4.0);
        double y = plot_h * i / 4.0;

        cr->move_to(-52, y + 4);
        cr->set_source_rgb(0.7, 0.7, 0.7);
        cr->set_font_size(9);
        auto txt = std::to_string(val);
        auto dot = txt.find('.');
        if (dot != std::string::npos)
            txt = txt.substr(0, dot + 3);
        cr->show_text(txt);
    }

    // Time axis
    double total_time = time_window_;
    for (int i = 0; i <= 5; ++i) {
        double t = total_time * i / 5.0;
        double x = plot_w * i / 5.0;
        cr->move_to(x, plot_h + 12);
        cr->set_source_rgb(0.7, 0.7, 0.7);
        cr->set_font_size(9);
        cr->show_text((std::to_string((int)t) + tr("graph_s")).c_str());
    }

    cr->restore();
}

void GraphWidget::draw_grid(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h)
{
    cr->save();
    cr->set_source_rgba(0.3, 0.3, 0.3, 0.5);
    cr->set_line_width(0.5);

    for (int i = 0; i <= 4; ++i) {
        double y = h * i / 4.0;
        cr->move_to(0, y);
        cr->line_to(w, y);
        cr->stroke();
    }

    for (int i = 0; i <= 5; ++i) {
        double x = w * i / 5.0;
        cr->move_to(x, 0);
        cr->line_to(x, h);
        cr->stroke();
    }

    cr->restore();
}

void GraphWidget::draw_series(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& s : series_) {
        if (s.data.size() < 2) continue;

        double rng = s.max_val - s.min_val;
        if (rng < 1e-10) rng = 1.0;

        size_t max_samples = static_cast<size_t>(time_window_ * sample_rate_);
        size_t n = s.data.size();

        cr->save();
        cr->set_source_rgba(
            s.color.get_red(), s.color.get_green(),
            s.color.get_blue(), 0.9);
        cr->set_line_width(1.5);

        for (size_t i = 0; i < n; ++i) {
            double x = w * (1.0 - static_cast<double>(n - i) / max_samples);
            double y = h * (s.max_val - s.data[i]) / rng;

            y = std::clamp(y, 0.0, static_cast<double>(h));

            if (i == 0)
                cr->move_to(x, y);
            else
                cr->line_to(x, y);
        }

        cr->stroke();
        cr->restore();
    }
}

void GraphWidget::draw_legend(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (series_.empty()) return;

    double x = w - 10;
    double y = 5;

    for (size_t i = series_.size(); i > 0; --i) {
        auto& s = series_[i - 1];

        cr->save();
        cr->set_source_rgba(0, 0, 0, 0.5);
        cr->rectangle(x - 65, y, 60, 14);
        cr->fill();

        cr->set_source_rgba(
            s.color.get_red(), s.color.get_green(),
            s.color.get_blue(), 1.0);
        cr->rectangle(x - 60, y + 3, 8, 8);
        cr->fill();

        cr->set_source_rgb(1, 1, 1);
        cr->set_font_size(9);
        cr->move_to(x - 48, y + 10);
        cr->show_text(s.name);

        cr->restore();
        y += 16;
    }
}

void GraphWidget::draw_markers(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (markers_.empty()) return;

    size_t max_samples = static_cast<size_t>(time_window_ * sample_rate_);
    if (max_samples == 0) return;

    for (const auto& m : markers_) {
        // Convert x_pos (data time) to pixel x
        double x = w * (m.x_pos / time_window_);
        if (x < 0 || x > w) continue;

        cr->save();

        // Vertical dashed line
        cr->set_source_rgba(m.color.get_red(), m.color.get_green(), m.color.get_blue(), 0.6);
        cr->set_line_width(1.0);
        cr->set_dash(std::vector<double>{4.0, 4.0}, 0);
        cr->move_to(x, 0);
        cr->line_to(x, h);
        cr->stroke();

        // Text label at top
        cr->set_dash(std::vector<double>(), 0);
        cr->set_source_rgba(m.color.get_red(), m.color.get_green(), m.color.get_blue(), 1.0);
        cr->set_font_size(9);

        // Background rect for readability
        Cairo::TextExtents extents;
        cr->get_text_extents(m.label, extents);
        cr->set_source_rgba(0, 0, 0, 0.6);
        cr->rectangle(x - 2, 0, extents.width + 4, 12);
        cr->fill();

        cr->set_source_rgba(m.color.get_red(), m.color.get_green(), m.color.get_blue(), 1.0);
        cr->move_to(x, 10);
        cr->show_text(m.label);

        cr->restore();
    }
}
