#ifndef BD226299_35E8_4860_AC3E_7E351700A4D7
#define BD226299_35E8_4860_AC3E_7E351700A4D7

#include "client/gui/widget.hh"

namespace gui
{
class Popup final : public WidgetBuilder<Popup> {
public:
    Popup& set_title(std::string_view title);
    Popup& set_message(std::string_view question);
    Popup& add_choice(std::string_view choice, std::function<void(void)> callback = {});

    virtual void layout(void) override;
    virtual void translate(void) override;

    void open(void);

private:
    struct Choice final {
        std::string label_key;
        std::string label;
        std::function<void(void)> callback;
    };

    std::string m_title_key {};
    std::string m_title {};

    std::string m_message_key {};
    std::string m_message {};

    std::vector<Choice> m_choices {};

    bool m_queued_open { false };
};
} // namespace gui

namespace gui
{
class InputPopup final : public WidgetBuilder<InputPopup> {
public:
    InputPopup& set_title(std::string_view title);
    InputPopup& add_input(std::string_view label, ImGuiInputTextFlags flags = 0, std::function<bool(std::string_view)> validator = {});
    InputPopup& on_submit(std::function<void(std::span<const std::string>)> callback);
    InputPopup& on_cancel(std::function<void(void)> callback);

    virtual void layout(void) override;
    virtual void translate(void) override;

    void open(void);

    std::string_view value(std::size_t index) const;
    void set_value(std::size_t index, std::string_view value);

private:
    struct Field final {
        std::string label_key;
        std::string label;
        std::string buffer_1;
        std::string buffer_2;
        ImGuiInputTextFlags flags;
        std::function<bool(std::string_view)> validator;
    };

    std::string m_title_key {};
    std::string m_title {};

    std::vector<Field> m_fields {};
    std::function<void(std::span<const std::string>)> m_submit {};
    std::function<void(void)> m_cancel {};

    std::string m_submit_label {};
    std::string m_cancel_label {};

    bool m_queued_open { false };
    bool m_focus_first { false };
};
} // namespace gui

#endif /* BD226299_35E8_4860_AC3E_7E351700A4D7 */
