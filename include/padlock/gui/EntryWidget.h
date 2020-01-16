#ifndef PADLOCK_GUI_ENTRY_H_
#define PADLOCK_GUI_ENTRY_H_

#include <codecvt>
#include <functional>
#include <locale>
#include <string>

#define GTKMM_DISABLE_DEPRECATED 1
#include <gdk/gdk.h>
#include <gtkmm.h>

namespace padlock {

namespace {

    std::string toUtf8(const std::u32string& str) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.to_bytes(str);
    }

} // namespace

class Entry : public Gtk::DrawingArea {
    std::string mData;
    std::size_t mCaretPosition;
    bool mIsPasswordEntry;
    std::string mCueBanner;

    struct {
        std::function<bool(Entry*)> onReturn;
        std::function<bool(Entry*)> onTab;
    } mCallbacks;

public:
    Entry()
        : mCaretPosition(0)
        , mIsPasswordEntry(false) {
        set_can_focus(true);
        set_size_request(200, 40);
        add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK);
    }

    void callOnReturn(std::function<bool(Entry*)>&& callback) { mCallbacks.onReturn = std::move(callback); }

    void callOnTab(std::function<bool(Entry*)>&& callback) { mCallbacks.onTab = std::move(callback); }

    const std::string& getText() const { return mData; }

    void setIsPassword(const bool isPassword) { mIsPasswordEntry = isPassword; }

    void clear() {
        mData.clear();
        mCaretPosition = 0;
        queue_draw();
    }

    void setCueBanner(const std::string& text) { mCueBanner = text; }

private:
    bool on_key_press_event(GdkEventKey* event) override {
        switch (event->keyval) {
        case 65288: // Backspace
            if (!mData.empty() && mCaretPosition != 0) {
                std::string::iterator curCharPos = mData.begin() + mCaretPosition - 1;
                mData.erase(curCharPos, curCharPos + 1);
                --mCaretPosition;
                queue_draw();
            }
            return true;
        case 65535: // Delete
            if (!mData.empty() && mCaretPosition < mData.size()) {
                std::string::iterator curCharPos = mData.begin() + mCaretPosition - 1;
                mData.erase(curCharPos + 1, curCharPos + 2);
                queue_draw();
            }
            return true;
        case 65361: // left arrow
            if (mCaretPosition != 0) {
                --mCaretPosition;
                queue_draw();
            }
            return true;
        case 65363: // right arrow
            if (mCaretPosition < mData.size()) {
                ++mCaretPosition;
                queue_draw();
            }
            return true;
        case 65289: // TAB
            if (mCallbacks.onTab) {
                return mCallbacks.onTab(this);
            }
            return false;
        case 65293: // Enter
            if (mCallbacks.onReturn) {
                return mCallbacks.onReturn(this);
            }
            return false;
        }

        const gunichar utfChar = gdk_keyval_to_unicode(event->keyval);
        const bool isPrint = g_unichar_isprint(utfChar);
        const bool ctrlKeyPressed = event->state & GDK_CONTROL_MASK;
        if (!isPrint || ctrlKeyPressed) {
            return true;
        }
        mData.insert(mCaretPosition, event->string);
        mCaretPosition += event->length;
        queue_draw();
        return true;
    }

    bool on_button_press_event(GdkEventButton*) override {
        grab_focus();
        return true;
    }

    void draw_rounded_path(const Cairo::RefPtr<Cairo::Context>& cr,
                           double x,
                           double y,
                           double width,
                           double height,
                           double radius) {
        const double degrees = 3.14159265 / 180.0;
        cr->begin_new_sub_path();
        cr->arc(x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
        cr->arc(x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
        cr->arc(x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
        cr->arc(x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
        cr->close_path();
    }

    void
    draw_text(const Cairo::RefPtr<Cairo::Context>& cr, const std::string& text, const int x, const int y) {
        Pango::FontDescription font;
        font.set_family("Monospace");
        font.set_weight(Pango::WEIGHT_NORMAL);
        auto layout = create_pango_layout(text);
        layout->set_font_description(font);
        int textWidth;
        int textHeight;
        layout->get_pixel_size(textWidth, textHeight);
        const int width = get_allocation().get_width();
        const int textOrigin = textWidth >= width ? x + width - textWidth : x;
        cr->move_to(textOrigin, y - textHeight / 2);
        layout->show_in_cairo_context(cr);

        if (has_focus()) {
            const int curPos = mIsPasswordEntry ? mCaretPosition * 3 : mCaretPosition;
            get_style_context()->render_insertion_cursor(
                cr, x, y - textHeight / 2, layout, curPos, Pango::DIRECTION_LTR);
        }
    }

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
        Gtk::Allocation allocation = get_allocation();
        const int width = allocation.get_width();
        const int height = allocation.get_height();

        draw_rounded_path(cr, 0, 0, width, height, 10);
        cr->clip();

        // Draw background
        const float opacity = has_focus() ? 0.5f : 0.1f;
        cr->set_source_rgba(0.5f, 0.5f, 0.5f, opacity);
        cr->paint();

        if (mData.empty() && !has_focus() && !mCueBanner.empty()) {
            cr->set_source_rgb(0.7, 0.7, 0.7);
            draw_text(cr, mCueBanner, 10, height / 2);
        } else if (!mData.empty()) {
            cr->set_source_rgb(0.0, 0.0, 0.0);
            const std::string passChar(
                toUtf8(std::u32string(mData.size(), 9679))); // 9679 is unicode dot character
            draw_text(cr, mIsPasswordEntry ? passChar : mData, 10, height / 2);
        }

        return true; // eat event
    }
};

} // namespace padlock

#endif // PADLOCK_GUI_ENTRY_H_
