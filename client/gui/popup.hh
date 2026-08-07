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

#endif /* BD226299_35E8_4860_AC3E_7E351700A4D7 */
