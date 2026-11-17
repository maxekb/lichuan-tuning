#pragma once
#include <gtkmm.h>
#include <string>

struct ParamRow : public Glib::Object {
    Glib::ustring id;
    Glib::ustring current;
    Glib::ustring default_val;
    Glib::ustring range;
    Glib::ustring unit;
    Glib::ustring desc;

    static Glib::RefPtr<ParamRow> create(
        const Glib::ustring& id_,
        const Glib::ustring& current_,
        const Glib::ustring& default_,
        const Glib::ustring& range_,
        const Glib::ustring& unit_,
        const Glib::ustring& desc_
    ) {
        return Glib::make_refptr_for_instance<ParamRow>(
            new ParamRow(id_, current_, default_, range_, unit_, desc_)
        );
    }

private:
    ParamRow(const Glib::ustring& id_,
             const Glib::ustring& current_,
             const Glib::ustring& default_,
             const Glib::ustring& range_,
             const Glib::ustring& unit_,
             const Glib::ustring& desc_)
        : Glib::Object()
        , id(id_)
        , current(current_)
        , default_val(default_)
        , range(range_)
        , unit(unit_)
        , desc(desc_)
    {}
};
