#include <gtkmm/application.h>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "gui_main_window.h"
#include "theme.h"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");

    for (int i = 1; i < argc; ++i) {
        if (argv[i] && strcmp(argv[i], "--lang") == 0 && i + 1 < argc && argv[i + 1]) {
            setenv("LANGUAGE", argv[i + 1], 1);
            // Remove --lang and its value from argv for GTK
            for (int j = i; j + 2 < argc; ++j)
                argv[j] = argv[j + 2];
            argc -= 2;
            break;
        }
    }

    // --no-splash: skip the splash (used for design iteration/testing)
    bool show_splash = true;
    for (int i = 1; i < argc; ++i) {
        if (argv[i] && strcmp(argv[i], "--no-splash") == 0) {
            show_splash = false;
            for (int j = i; j + 1 < argc; ++j)
                argv[j] = argv[j + 1];
            --argc;
            break;
        }
    }

    // If --lang was not passed, try settings.json
    {
        const char* home = getenv("HOME");
        if (home) {
            std::string settings_path = std::string(home) + "/.config/servo_tuning/settings.json";
            std::ifstream f(settings_path);
            if (f.is_open()) {
                std::string content((std::istreambuf_iterator<char>(f)),
                                      std::istreambuf_iterator<char>());
                auto pos = content.find("\"lang\"");
                if (pos != std::string::npos) {
                    auto colon = content.find(':', pos);
                    if (colon != std::string::npos) {
                        auto start = content.find('"', colon + 1);
                        if (start != std::string::npos) {
                            auto end = content.find('"', start + 1);
                            if (end != std::string::npos) {
                                std::string lang = content.substr(start + 1, end - start - 1);
                                if (lang == "ru" || lang == "zh" || lang == "en") {
                                    char* existing = getenv("LANGUAGE");
                                    if (!existing || strlen(existing) == 0)
                                        setenv("LANGUAGE", lang.c_str(), 1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    auto app = Gtk::Application::create();

    app->signal_startup().connect([app, show_splash] {
        auto settings = Gtk::Settings::get_default();
        settings->set_property("gtk-application-prefer-dark-theme", true);
        // The system icon theme (breeze) provides no window-close/-minimize/
        // -maximize symbolic glyphs for GTK4 apps, so the CSD title buttons
        // render as invisible empty areas. Adwaita ships the full symbolic
        // set and its symbolics honor the CSS `color` of theme.h.
        settings->set_property("gtk-icon-theme-name", Glib::ustring("Adwaita"));
        apply_app_theme();

        auto window = Gtk::make_managed<MainWindow>(show_splash);
        app->add_window(*window);
        window->present();
    });

    return app->run();
}
