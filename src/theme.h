#pragma once
#include <gtkmm/cssprovider.h>
#include <gdkmm/display.h>
#include <cstdio>

// Application-wide dark theme, embedded as a raw string so the binary has no
// runtime dependency on a stylesheet file next to it.
//
// Palette: derived from the GNOME/libadwaita DARK color scheme — the native
// Linux look for engineering tooling: window #242424, view/canvas #1e1e1e,
// headerbar #303030, controls #383838, accent #3584e4, semantic status
// green #33d17a / yellow #f6d32d / red #ff4d37. Color is reserved for
// status and interaction state only (industrial HMI discipline).
// Numeric readouts use a monospace face with tabular figures ("tnum") so
// digits align in the parameter table and DRO displays.
//
// IMPORTANT: use the `background` shorthand (never bare background-color) for
// any surface we recolor: the system GTK theme (Breeze) paints buttons,
// stackswitcher tabs, column headers and comboboxes via background-image
// gradients, and a bare background-color rule stays hidden underneath them.
//
// The splash screen and About dialog also install small per-widget CSS
// providers (APPLICATION priority, widget scope) which override these rules.
inline constexpr const char* APP_THEME_CSS = R"CSS(
window {
    background: #242424;
    color: #e8e8e8;
}

headerbar {
    background: #303030;
    border-bottom: 1px solid #3c3c3c;
    min-height: 48px;
}

/* Window title: GTK4 core has NO `windowtitle` node (that is libadwaita's
   AdwWindowTitle) — a `headerbar windowtitle` rule is dead code and the
   title keeps Breeze's near-black theme_titlebar_foreground. The real nodes
   are the labels with .title/.subtitle style classes inside windowhandle. */
headerbar .title,
headerbar .subtitle {
    color: #ffffff;
    font-weight: bold;
    font-size: 16px;
}

/* IMPORTANT: exclude .titlebutton! A bare `headerbar button` rule also
   matches the window-control buttons and, with the Breeze GTK4 theme active,
   wipes the background-image glyphs that draw close/minimize/maximize
   (verified with a bare-HeaderBar probe: buttons render only when untouched).
   Only our own header buttons (menu) get restyled here. */
headerbar button:not(.titlebutton) {
    background: none;
    border: none;
    color: #d6d6d6;
    min-width: 40px;
    min-height: 40px;
}

headerbar button:not(.titlebutton):hover {
    background: #3d3d3d;
    border-radius: 8px;
}

/* StackSwitcher tabs — pills; the checked pill floats out of the chrome */
stackswitcher button {
    background: none;
    border: none;
    color: #a9a9a9;
    padding: 8px 22px;
    margin: 4px 2px;
    border-radius: 8px;
    font-size: 13px;
    font-weight: 600;
}

stackswitcher button:hover {
    background: #2f2f2f;
    color: #e8e8e8;
}

stackswitcher button:checked {
    background: #383838;
    color: #ffffff;
}

/* Status bar readout — control-panel green on dark */
.status-label {
    color: #33d17a;
    font-size: 12px;
    font-family: monospace;
    font-feature-settings: "tnum";
}

/* DRO values (monitor page) — dark instrument glass */
.dro-value {
    font-size: 28px;
    font-family: monospace;
    font-feature-settings: "tnum";
    font-weight: bold;
    color: #ffffff;
}

.dro-label {
    font-size: 11px;
    color: #8d8d8d;
}

.dro-status {
    font-size: 24px;
    font-family: monospace;
    font-feature-settings: "tnum";
}

.dro-frame {
    border: 1px solid #3a3a3a;
    border-radius: 8px;
    background: #161616;
    padding: 8px;
}

/* Parameter table (ColumnView) — the canvas: darkest surface.
   Node name for the header differs across GTK 4.x, cover all spellings. */
columnview {
    background: #1e1e1e;
    font-feature-settings: "tnum";
}

columnview columnview-header,
columnview column-header,
columnview column_header {
    background: #242424;
    border-bottom: 1px solid #3a3a3a;
}

columnview columnview-header button,
columnview column-header button,
columnview column_header button {
    background: none;
    border: none;
    color: #b5b5b5;
    font-weight: 600;
    font-size: 12px;
}

columnview columnview-header button:hover,
columnview column-header button:hover,
columnview column_header button:hover {
    background: #2c2c2c;
    color: #e8e8e8;
}

columnview listview row {
    background: #1e1e1e;
    border-bottom: 1px solid #242424;
    color: #e8e8e8;
    padding: 2px 0;
}

columnview listview row:hover {
    background: #2a2a2a;
}

columnview listview row:selected,
columnview listview row:selected:hover {
    background: #3584e4;
    color: #ffffff;
}

/* ComboBoxText / Dropdowns */
combobox button,
combobox button:disabled {
    background: #383838;
    border: 1px solid #4b4b4b;
    border-radius: 8px;
    color: #e8e8e8;
}

combobox button:hover {
    background: #444444;
}

popover {
    background: #383838;
    border: 1px solid #4a4a4a;
    border-radius: 10px;
}

/* Popover/combobox item TEXT must be colored on the label directly: our row
   color only INHERITS into the label, and Breeze colors EVERYTHING via a
   universal `* { color: #232629 }` rule (gtk.css:189 of the installed theme)
   — a direct rule on the node is the only thing that beats it. */
popover label {
    color: #e8e8e8;
}

popover row:selected label,
popover row:selected:hover label {
    color: #ffffff;
}

/* GtkComboBox popup IS a `popover` in GTK4 (the docs' "window.popup" node
   tree is stale GTK3 text; verified with a probe build: our `popover`
   background rule darkens it). But its items are GtkCellView widgets that
   render text via Pango render_layout — there is NO label node to target,
   so name-based selectors (label/cellview/cell) all missed. Universal
   descendant rule: direct color on every node inside popovers. Scope is
   popovers only (combo popups + our ≡ menu), cannot leak elsewhere. */
popover * {
    color: #e8e8e8;
}

popover row:selected,
popover row:selected *,
popover *:selected {
    color: #ffffff;
}

popover contents,
popover listview {
    background: #383838;
}

popover listview row {
    background: none;
    color: #e8e8e8;
    border-radius: 6px;
}

popover listview row:hover {
    background: #454545;
}

popover listview row:selected {
    background: #3584e4;
    color: #ffffff;
}

/* Entry */
entry {
    background: #1e1e1e;
    border: 1px solid #454545;
    border-radius: 8px;
    color: #f0f0f0;
    padding: 6px 12px;
    font-feature-settings: "tnum";
}

entry:focus {
    background: #1e1e1e;
    border: 1px solid #3584e4;
}

/* Buttons. :not(.titlebutton) everywhere: the `background:` shorthand resets
   background-image, and the Breeze GTK4 theme draws the window-control glyphs
   AS background images — a plain `button {}` rule wipes them out (verified by
   probe: buttons exist but render as empty boxes). */
button:not(.titlebutton) {
    background: #383838;
    border: 1px solid #4b4b4b;
    border-radius: 8px;
    color: #e8e8e8;
    padding: 6px 16px;
}

button:not(.titlebutton):hover {
    background: #444444;
}

/* Disabled buttons must look disabled (save-in-flight, read-only, dead bus) */
button:not(.titlebutton):disabled,
button:not(.titlebutton):disabled:hover {
    background: #2e2e2e;
    border-color: #3a3a3a;
    color: #6a6a6a;
}

/* Own text-glyph window controls (see gui_main_window.cpp build_menu):
   flat on the headerbar, red hover over close — standard desktop semantics.
   Text glyphs, so no icon theme / Breeze asset path can break them. */
headerbar button.titlebtn {
    background: none;
    border: none;
    color: #d6d6d6;
    min-width: 30px;
    min-height: 30px;
    padding: 0;
    margin: 0 1px;
    border-radius: 6px;
    font-size: 15px;
}

headerbar button.titlebtn:hover {
    background: #3d3d3d;
    color: #ffffff;
}

headerbar button.titlebtn.btn-win-close:hover {
    background: #c01c28;
    color: #ffffff;
}

/* Semantic button colors */
button.btn-primary,
button.btn-primary:hover {
    background: #3584e4;
    border-color: #3584e4;
    color: #ffffff;
}

button.btn-primary:hover {
    background: #4693f0;
}

button.btn-success,
button.btn-success:hover {
    background: #26a269;
    border-color: #26a269;
    color: #ffffff;
}

button.btn-success:hover {
    background: #2ab876;
}

button.btn-danger,
button.btn-danger:hover {
    background: #c01c28;
    border-color: #c01c28;
    color: #ffffff;
}

button.btn-danger:hover {
    background: #d0232e;
}

button.btn-running,
button.btn-running:hover {
    background: #c64600;
    border-color: #c64600;
    color: #ffffff;
}

button.btn-running:hover {
    background: #d94e04;
}

/* Scale */
scale trough {
    background: #383838;
    border-radius: 6px;
    min-height: 6px;
}

scale highlight {
    background: #3584e4;
    border-radius: 6px;
}

scale slider {
    background: #e0e0e0;
    border-radius: 12px;
    min-width: 22px;
    min-height: 22px;
}

/* Dialogs */
dialog,
window.dialog {
    background: #242424;
    color: #e8e8e8;
}

/* Monitor fault history is a Gtk::ColumnView (AT-SPI lies: it reports the
   same "tree table" role as GtkTreeView). Its empty body must be colored
   explicitly; the class makes this table addressable regardless of theme
   node quirks. */
columnview.fault-list,
columnview.fault-list listview,
columnview.fault-list listview > row,
columnview.fault-list .view {
    background: #1e1e1e;
    color: #e8e8e8;
}

/* GtkTreeView (Monitor fault history) — official node is `treeview.view`;
   without a rule Breeze paints it with its light @theme_view background
   (a white slab under the dark theme). */
treeview.view {
    background: #1e1e1e;
    color: #e8e8e8;
}

treeview.view:hover {
    background: #2a2a2a;
}

treeview.view:selected,
treeview.view:selected:focus {
    background: #3584e4;
    color: #ffffff;
}

treeview header button {
    background: #242424;
    border: none;
    border-bottom: 1px solid #3a3a3a;
    color: #b5b5b5;
    font-weight: 600;
    font-size: 12px;
}

treeview header button:hover {
    background: #2c2c2c;
    color: #e8e8e8;
}

/* GTK4 TreeView rows are internal `row` gizmos (GtkListBase), the view node
   rule alone leaves Breeze's light view background showing through the row
   area. Cover the whole node family. */
treeview.view row,
treeview.view row:selected,
treeview.view cell,
scrolledwindow viewport,
scrolledwindow > viewport > treeview.view {
    background: #1e1e1e;
    color: #e8e8e8;
}

treeview.view row:hover {
    background: #2a2a2a;
}

treeview.view row:selected,
treeview.view row:selected cell {
    background: #3584e4;
    color: #ffffff;
}

/* Scrollbars */
scrollbar {
    background: #1e1e1e;
    border: none;
}

scrollbar slider {
    background: #4a4a4a;
    border-radius: 6px;
    min-width: 10px;
}

scrollbar slider:hover {
    background: #5c5c5c;
}

/* Frame */
frame {
    border: 1px solid #3f3f3f;
    border-radius: 8px;
    padding: 12px;
}

frame > label {
    color: #e8e8e8;
}

/* Stack */
stack {
    background: #242424;
}

/* ProgressBar */
progressbar trough {
    background: #383838;
    border-radius: 6px;
    min-height: 12px;
}

progressbar progress {
    background: #3584e4;
    border-radius: 6px;
}

/* SpinButton */
spinbutton {
    background: #1e1e1e;
    border: 1px solid #454545;
    border-radius: 8px;
    color: #f0f0f0;
    font-feature-settings: "tnum";
}

/* GtkSpinButton/GtkEntry hold their digits in a child `text` node — Breeze
   colors `text` directly and our spinbutton color only INHERITS (loses to
   any direct rule regardless of priority). Color the node itself. */
spinbutton text,
entry > text,
textview text {
    color: #f0f0f0;
    background: transparent;
    caret-color: #f0f0f0;
}

spinbutton button {
    background: #383838;
    border: none;
    color: #d6d6d6;
}

spinbutton button:hover {
    background: #454545;
}

/* Stats / meta labels */
.stats-label {
    color: #9a9a9a;
    font-size: 12px;
    font-family: monospace;
}

.changes-label {
    color: #f6d32d;
    font-size: 12px;
}

.apply-mode {
    color: #8a8a8a;
    font-size: 11px;
    font-style: italic;
}

.section-header {
    color: #78aeed;
    font-size: 14px;
    font-weight: bold;
}

.ecat-op {
    color: #33d17a;
    font-weight: bold;
    font-size: 12px;
    padding: 2px 6px;
}

.ecat-warn {
    color: #f6d32d;
    font-weight: bold;
    font-size: 12px;
    padding: 2px 6px;
}

.ecat-offline {
    color: #ff4d37;
    font-weight: bold;
    font-size: 12px;
    padding: 2px 6px;
}

/* Inline buttons inside table cells */
.cell-arrow-btn,
.cell-arrow-btn:hover,
.cell-edit-btn,
.cell-edit-btn:hover {
    min-width: 0;
    min-height: 0;
    padding: 0;
    margin: 0;
    border: none;
    border-radius: 4px;
    background: none;
    color: currentColor;
}

.cell-arrow-btn {
    border: 1px solid alpha(currentColor, 0.3);
}

.cell-edit-btn {
    padding: 2px 6px;
    font-size: 14px;
}

.cell-arrow-btn:hover,
.cell-edit-btn:hover {
    background: alpha(currentColor, 0.12);
}

/* Parameter edit dialog */
.edit-dialog-title {
    font-weight: bold;
    font-size: 18px;
}
.edit-value {
    font-size: 28px;
    font-weight: bold;
    font-family: monospace;
    font-feature-settings: "tnum";
}
.edit-unit {
    font-size: 18px;
    color: #9a9a9a;
}
.edit-range {
    font-size: 13px;
    color: #8d8d8d;
}
.edit-desc {
    font-size: 14px;
    font-style: italic;
    color: #c0c0c0;
}
.edit-error {
    font-size: 13px;
    font-weight: bold;
    color: #ff4d37;
}
.edit-btn {
    min-width: 54px;
    min-height: 48px;
    font-size: 18px;
    padding: 8px 16px;
    border-radius: 8px;
}

/* Enum value descriptions in the edit dialog */
.edit-value-descs {
    border: 1px solid #3a3a3a;
    border-radius: 8px;
    background: #1e1e1e;
    padding: 6px;
}
.edit-value-desc-item {
    font-size: 12px;
    font-family: monospace;
    font-feature-settings: "tnum";
    color: #c0c0c0;
}

.cell-label {
    padding: 2px 4px;
}
)CSS";

// Must be called after the Gdk display exists (e.g. from signal_startup).
inline void apply_app_theme() {
    auto display = Gdk::Display::get_default();
    if (!display) {
        fprintf(stderr, "[Theme] no Gdk display, theme not applied\n");
        return;
    }
    auto provider = Gtk::CssProvider::create();
    try {
        provider->load_from_data(APP_THEME_CSS);
    } catch (const std::exception& e) {
        fprintf(stderr, "[Theme] CSS parse error: %s\n", e.what());
        return;
    }
    // USER priority, not APPLICATION: KDE generates ~/.config/gtk-4.0/gtk.css
    // (+colors.css) and ships a Breeze GTK4 theme that otherwise win over an
    // APPLICATION-priority provider (e.g. dark Breeze title text on our dark
    // headerbar). We register at runtime, so at equal priority we still win.
    Gtk::StyleContext::add_provider_for_display(
        display, provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}
