#pragma once
#include <gtkmm.h>
#include <gtkmm/drawingarea.h>
#include <vector>
#include <deque>
#include <mutex>
#include <cairomm/context.h>
#include <cairomm/surface.h>

struct GraphSeries {
    std::deque<double> data;
    Gdk::RGBA color;
    double min_val = 0.0;
    double max_val = 100.0;
    Glib::ustring name;
};

struct GraphMarker {
    double x_pos;           // position on X axis (in data units, e.g., time in seconds)
    Glib::ustring label;    // text annotation
    Gdk::RGBA color;        // line and text color
};

class GraphWidget : public Gtk::DrawingArea {
public:
    GraphWidget();
    ~GraphWidget() override = default;

    void add_series(const Glib::ustring& name, const Gdk::RGBA& color,
                    double min_val, double max_val);
    void append_data(size_t series_idx, double value);
    void set_time_window(double seconds);
    void clear();
    void set_series_name(size_t series_idx, const Glib::ustring& name);

    void add_marker(const GraphMarker& marker);
    void clear_markers();

 private:
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h);
    void draw_grid(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h);
    void draw_series(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h);
    void draw_legend(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h);
    void draw_markers(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h);

    std::vector<GraphSeries> series_;
    std::vector<GraphMarker> markers_;
    double time_window_ = 10.0;
    double sample_rate_ = 10.0;
    mutable std::mutex mutex_;
    bool dirty_ = true;
};
