#ifndef F5A3A7CE_B769_4286_A715_E64D2971E7A5
#define F5A3A7CE_B769_4286_A715_E64D2971E7A5

#include "client/gui/widget.hh"

namespace gui
{
template<typename Derived>
class ContainerBuilder : public WidgetBuilder<Derived> {
public:
    Derived& add_child(Widget& child, int priority = 0);

    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    struct Item final {
        Widget* widget;
        int priority;
    };

    std::vector<Item> m_children {};
};
} // namespace gui

namespace gui
{
class Container final : public ContainerBuilder<Container> {};
} // namespace gui

template<typename Derived>
Derived& gui::ContainerBuilder<Derived>::add_child(Widget& child, int priority)
{
    auto it = std::ranges::upper_bound(m_children, priority, std::less {}, &Item::priority);

    Item item {};
    item.widget = &child;
    item.priority = priority;

    m_children.insert(it, std::move(item));

    return this->self();
}

template<typename Derived>
void gui::ContainerBuilder<Derived>::layout(void)
{
    for(auto& child : m_children) {
        if(child.widget->visible()) {
            child.widget->layout();
        }
    }
}

template<typename Derived>
void gui::ContainerBuilder<Derived>::translate(void)
{
    for(auto& child : m_children) {
        child.widget->translate();
    }
}

#endif /* F5A3A7CE_B769_4286_A715_E64D2971E7A5 */
